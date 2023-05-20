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

void register_user(int sockfd, string local_host, string url, string content_type, ordered_json jwt_token) {
    char* request;
    char* response;
    ordered_json credentials;

    // read the username and password from the user
    credentials = read_user_credentials();
    request = compute_json_post_request(local_host, url, content_type, credentials, NULL, 0, jwt_token);
    printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    // free the request and response
    free(request);
    free(response);
}

void login_user(int sockfd, string local_host, string url, string content_type, ordered_json jwt_token, char **coockies) {
    // check if the user is already logged in
    if (coockies != NULL && coockies[0][0] != '\0') {
        printf("Already logged in\n");
        return;
    }
    char *request;
    char *response;
    ordered_json credentials;
    // read the username and password from the user
    credentials = read_user_credentials();
    request = compute_json_post_request(local_host, url, content_type, credentials, NULL, 0, jwt_token);
    printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    printf("%s\n", response);
    if (strlen(coockies[0]) == 0) {
        extract_coockies(response, coockies);
    }
    // free the request and response
    free(request);
    free(response);
    return;
}

ordered_json enter_library(int sockfd, string local_host, string url, ordered_json jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("You are not logged in\n");
        jwt_token.clear();
        return jwt_token;
    }
    char *request;
    char *response;
    request = compute_get_request(local_host, url, "", coockies, 1, jwt_token);
    printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    printf("%s\n", response);
    // extract JWT from the response
    jwt_token = ordered_json::parse(basic_extract_json_response(response));
    // free the request and response
    free(request);
    free(response);
    return jwt_token;
}

void get_books(int sockfd, string local_host, string url, ordered_json jwt_token, char **coockies) {
    // if the user is not logged in, send an error message
    if (coockies == NULL || coockies[0][0] == '\0') {
        printf("You are not logged in\n");
        jwt_token.clear();
        return;
    }
    char *request;
    char *response;
    request = compute_get_request(local_host, url, "", coockies, 1, jwt_token);
    printf("%s\n", request);
    send_to_server(sockfd, request);
    response = receive_from_server(sockfd);
    printf("%s\n", response);

    // get the error message if the user doen't have access to the library
    char *error_message = strstr(response, "error");
    if (error_message != NULL) {
        printf("You don't have access to the library\n");
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
    free(request);
    free(response);
    return;
}