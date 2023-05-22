#include <iostream>
#include <string>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <nlohmann/json.hpp>
#include "helpers.h"
#include "buffer.h"
#include "requests.h"

using namespace std;
using ordered_json = nlohmann::ordered_json;

void register_user(int sockfd, string local_host, string url,
                   string content_type, ordered_json *jwt_token) {
    char* request;
    char* response;
    ordered_json credentials;

    // check if the user is already logged in
    if (!jwt_token->empty()) {
        printf("400 - Bad Request - Already logged in\n");
        return;
    }

    // read the username and password from the user
    credentials = read_user_credentials();
    // check if the username or password are wrong
    if (credentials.is_null()) {
        printf("400 - Bad Request - Invalid username or password\n");
        return;
    }
    request = compute_json_post_request(local_host, url, content_type, 
                                        credentials, NULL, 0, *jwt_token);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);

    // check if the user is already registered
    char *error_message = strstr(response, "\"error\":\"The username");
    if (error_message != NULL) {
        printf("400 - Bad Request - User already registered\n");
        free(request);
        free(response);
        return;
    }
    
    printf("200 - OK - User registered successfully\n");
    // free the request and response
    free(request);
    free(response);
}

void login_user(int sockfd, string local_host, string url, string content_type,
                ordered_json *jwt_token, char **coockies) {
    // check if the user is already logged in
    if (coockies != NULL && coockies[0][0] != '\0') {
        printf("400 - Bad Request - Already logged in\n");
        return;
    }
    char *request;
    char *response;
    ordered_json credentials;
    // read the username and password from the user
    credentials = read_user_credentials();
    // check if the username or password are wrong
    if (credentials.is_null()) {
        printf("400 - Bad Request - Invalid username or password\n");
        return;
    }
    request = compute_json_post_request(local_host, url, content_type,
                                        credentials, NULL, 0, *jwt_token);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);

    // check if the username or password are wrong
    char *error_fake_user = strstr(response, "\"error\":\"No account with this username!\"");
    char *error_bad_credentials = strstr(response, "\"error\":\"Credentials are not good!\"");

    if (error_bad_credentials != NULL || error_fake_user != NULL) {
        printf("400 - Bad Request - Wrong username or password\n");
        free(request);
        free(response);
        return;
    }

    if (strlen(coockies[0]) == 0) {
        extract_coockies(response, coockies);
    }

    printf("200 - OK - Logged in successfully\n");
    // free the request and response
    free(request);
    free(response);
    return;
}

ordered_json enter_library(int sockfd, string local_host, string url,
                           ordered_json *jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("400 - Bad Request - You are not logged in\n");
        jwt_token->clear();
        return *jwt_token;
    }
    char *request;
    char *response;
    request = compute_get_request(local_host, url, "", coockies, 1, *jwt_token);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    // extract JWT from the response
    *jwt_token = ordered_json::parse(basic_extract_json_response(response));
    // free the request and response
    free(request);
    free(response);
    return *jwt_token;
}

void get_library(int sockfd, string local_host, string url,
                 ordered_json *jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("400 - Bad Request - You are not logged in\n");
        jwt_token->clear();
        return;
    }
    char *request;
    char *response;
    request = compute_get_request(local_host, url, "", coockies, 1, *jwt_token);
    // printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    // printf("%s\n", response);

    // get the error message if the user doen't have access to the library
    char *error_message = strstr(response, "error");
    if (error_message != NULL) {
        printf("400 - Bad Request - You don't have access to the library\n");
            free(request);
        free(response);
        return;
    }

    // if the library is empty, return an error message
    char *empty_library = strstr(response, "[]");
    char *content_length = strstr(response, "Content-Length: 2");
    if (empty_library != NULL && content_length != NULL) {
        printf("200 - OK - The library is empty\n");
        free(request);
        free(response);
        return;
    }
    
    // format and print the response to the user
    char* books_str = basic_extract_json_response(response);
    
    // add a [ at the beginning of the string
    char* aux = (char *)calloc(10000, sizeof(char));
    aux[0] = '[';
    strcat(aux, books_str);
    books_str = aux;
    // print the json response
    ordered_json books = ordered_json::parse(books_str);
    cout << books.dump(4) << endl;    

    // free the request and response
    free(aux);
    free(request);
    free(response);
    return;
}

void get_book(int sockfd, string local_host, string url,
              ordered_json *jwt_token, char **coockies) {
    // read the book id from the user
    printf("id=");
    char *book_id = (char *)calloc(100, sizeof(char));
    scanf("%s", book_id);

    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("400 - Bad Request - You are not logged in\n");
        jwt_token->clear();
        free(book_id);
        return;
    }
    char *request;
    char *response;


    // create the url for the request
    url = url + "/" + book_id;
    // check if the book id is a number
    if (strspn(book_id, "0123456789") != strlen(book_id)) {
        printf("400 - Bad Request - Invalid id\n");
        free(book_id);
        return;
    }
    free(book_id);
    request = compute_get_request(local_host, url, "", coockies, 1, *jwt_token);
    // printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    // printf("%s\n", response);

    // get the error message if the user doen't have access to the library
    char *error_authorization = strstr(response, "\"error\":\"Authorization header is missing!\"");
    if (error_authorization != NULL) {
        printf("400 - Bad Request - You don't have access to the library\n");
        free(request);
        free(response);
        return;
    }
    
    char *error_wrong_id = strstr(response, "\"error\":\"No book was found!\"");
    if (error_wrong_id != NULL) {
        printf("400 - Bad Request - No book was found with this id\n");
        free(request);
        free(response);
        return;
    }
    // format and print the response to the user
    char* book_str = basic_extract_json_response(response);
    // print the json response
    ordered_json book = ordered_json::parse(book_str);
    cout << book.dump(4) << endl;    

    // free the request and response
    free(request);
    free(response);
    return;
}

void add_book(int sockfd, string local_host, string url, string content_type,
              ordered_json *jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("400 - Bad Request - You are not logged in\n");
        jwt_token->clear();
        return;
    }
    char *request;
    char *response;
    // check if the jwt token is empty
    if (jwt_token->empty()) {
        printf("400 - Bad Request - You don't have access to the library\n");
        return;
    }
    ordered_json book;
    // read the book details from the user
    book = read_book_details();
    if (book.is_null()) {
        printf("400 - Bad Request - Invalid page number\n");
        return;
    }
    request = compute_json_post_request(local_host, url, content_type,
                                        book, coockies, 1, *jwt_token);
    // printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    // printf("%s\n", response);

    // get the error message if the user doen't have access to the library
    char *error_authorization = strstr(response, "\"error\":\"Authorization header is missing!\"");
    if (error_authorization != NULL) {
        printf("400 - Bad Request - You don't have access to the library\n");
        free(request);
        free(response);
        return;
    }
    
    printf("200 - OK - Book added successfully\n");

    // free the request and response
    free(request);
    free(response);
    return;
}

void delete_book(int sockfd, string local_host, string url,
                 ordered_json *jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("400 - Bad Request - You are not logged in\n");
        jwt_token->clear();
        return;
    }
    char *request;
    char *response;
    printf("id=");

    char *book_id = (char *)calloc(100, sizeof(char));
    
    // read the book id from the user
    scanf("%s", book_id);
    
    // create the url for the request
    url = url + "/" + book_id;
    // check if the book id is a number
    if (strspn(book_id, "0123456789") != strlen(book_id)) {
        printf("400 - Bad Request - Invalid id\n");
        free(book_id);
        return;
    }
    free(book_id);
    request = compute_delete_request(local_host, url, *jwt_token);

    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);

    // get the error message if the user doen't have access to the library
    char *error_authorization = strstr(response, "\"error\":\"Authorization header is missing!\"");
    if (error_authorization != NULL) {
        printf("400 - Bad Request - You don't have access to the library\n");
        free(request);
        free(response);
        return;
    }
    
    char *error_wrong_id = strstr(response, "\"error\":\"No book was deleted!\"");
    if (error_wrong_id != NULL) {
        printf("400 - Bad Request - No book was found with this id\n");
        free(request);
        free(response);
        return;
    }

    printf("200 - OK - Book deleted successfully\n");

    // free the request and response
    free(request);
    free(response);
    return;
}

void logout_user(int sockfd, string local_host, string url,
                 ordered_json *jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("400 - Bad Request - You are not logged in\n");
        jwt_token->clear();
        return;
    }
    char *request;
    char *response;
    request = compute_get_request(local_host, url, "", coockies, 1, *jwt_token);
    // printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    // printf("%s\n", response);
    
    // reset the jwt token
    jwt_token->clear();

    // reset the coockies
    for (int i = 0; i < MAX_COOCKIES; i++) {
        memset(coockies[i], 0, LINELEN);
    }

    printf("200 - OK - Logged out successfully\n");

    // free the request and response
    free(request);
    free(response);
    return;
}