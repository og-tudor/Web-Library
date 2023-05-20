#include <iostream>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "commands.h"
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using ordered_json = nlohmann::ordered_json;

#define PRINT_RESPONSE "============"

// #define LOCALHOST "34.254.242.81:8080"
// #define X_FORM "application/x-www-form-urlencoded"


int main() {
    string local_host = "34.254.242.81:8080";
    // get just the host_ip from local_host
    string host_ip = local_host.substr(0, local_host.find(":"));
    string x_form = "application/x-www-form-urlencoded";
    string json_form = "application/json";
    string book_id;
    char *command = (char *)calloc(100, sizeof(char));
    char *garbage = (char *)calloc(100, sizeof(char));
    char *request;
    char *response;
    ordered_json jwt_token;
    int sockfd;
    char **coockies = (char **)calloc(MAX_COOCKIES, sizeof(char *));
    for (int i = 0; i < MAX_COOCKIES; i++) {
        coockies[i] = (char *)calloc(LINELEN, sizeof(char));
    }

    ordered_json credentials;
    sockfd = open_connection(host_ip, 8080, AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        return -1;
    }

    while(1) {
        printf("==================New_Command==================\n");

        scanf("%s", command);
        fgets(garbage, 100, stdin);
        sockfd = open_connection(host_ip, 8080, AF_INET, SOCK_STREAM, 0);
        // While the user doesn t input exit, keep receiving commands
        if (strcmp(command, "exit") == 0) {
            free(command);
            free(garbage);
            printf("freeing coockies\n");
            for (int i = 0; i < MAX_COOCKIES; i++) {
                free(coockies[i]);
            }
            free(coockies);
            // close the socket
            close_connection(sockfd);
            break;
        }

        if (strcmp(command, "register") == 0) {
            register_user(sockfd, local_host, "/api/v1/tema/auth/register", json_form, jwt_token);
            // read the username and password from the user
            // credentials = read_user_credentials();
            // request = compute_json_post_request(local_host, "/api/v1/tema/auth/register", json_form, credentials, NULL, 0, jwt_token);
            // printf("%s\n", request);
            // send_to_server(sockfd, request);
            // response = receive_from_server(sockfd);

            // printf("%s\n", response);
            // // reset the credentials
            // credentials.clear();
            // free(request);
            // free(response);
        }

        if (strcmp(command, "login") == 0) {
            // read the username and password from the user
            
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                printf("%sAlreadyLoggedIn%s\n", PRINT_RESPONSE, PRINT_RESPONSE);
                // request = compute_json_post_request(local_host, "/api/v1/tema/auth/login", json_form, credentials, coockies, 1);

            } else {
                credentials = read_user_credentials();
                request = compute_json_post_request(local_host, "/api/v1/tema/auth/login", json_form, credentials, NULL, 0, jwt_token);
                printf("%s\n", request);
                send_to_server(sockfd, request);
                free(request);
                response = receive_from_server(sockfd);
                printf("%s\n", response);

                // extract the coockies from the response
                if (strlen(coockies[0]) == 0) {
                    extract_coockies(response, coockies);
                    printf("%sCoockiesExtracted%s\n", PRINT_RESPONSE, PRINT_RESPONSE);
                }
                // reset the credentials
                credentials.clear();
                free(response);
            }
        }

        if (strcmp(command, "enter_library") == 0) {
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                request = compute_get_request(local_host, "/api/v1/tema/library/access", "", coockies, 1, jwt_token);
            } else {
                request = compute_get_request(local_host, "/api/v1/tema/library/access", "", NULL, 0, jwt_token);
            }
            printf("%s\n", request);
            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            // extract JWT from the response
            jwt_token = ordered_json::parse(basic_extract_json_response(response));
            printf("%sJWTExtracted%s\n", PRINT_RESPONSE, PRINT_RESPONSE);
            // coockies = extract_coockies(response);

            free(request);
            free(response);
        }

        if (strcmp(command, "get_books") == 0) {
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                request = compute_get_request(local_host, "/api/v1/tema/library/books", "", coockies, 1, jwt_token);
            } else {
                request = compute_get_request(local_host, "/api/v1/tema/library/books", "", NULL, 0, jwt_token);
            }
            printf("%s\n", request);
            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            free(request);
            free(response);
        }

        if (strcmp(command, "get_book") == 0) {
            // reading the book_id from the user
            char *book_id_input = (char *)calloc(LINELEN, sizeof(char));
            printf("id=");
            scanf("%s", book_id_input);
            string book_id = "/api/v1/tema/library/books/" + string(book_id_input);
            free(book_id_input);
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                request = compute_get_request(local_host, book_id, "", coockies, 1, jwt_token);
            } else {
                request = compute_get_request(local_host, book_id, "", NULL, 0, jwt_token);
            }
            printf("%s\n", request);
            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            free(request);
            free(response);
        }

        if (strcmp(command, "add_book") == 0) {
            // read the book details from the user
            ordered_json book_details = read_book_details();
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                request = compute_json_post_request(local_host, "/api/v1/tema/library/books", json_form, book_details, coockies, 1, jwt_token);
            } else {
                request = compute_json_post_request(local_host, "/api/v1/tema/library/books", json_form, book_details, NULL, 0, jwt_token);
            }
            printf("%s\n", request);
            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            free(request);
            free(response);
        }

        if (strcmp(command, "delete_book") == 0) {
            // reading the book_id from the user
            char *book_id_input = (char *)calloc(LINELEN, sizeof(char));
            printf("id=");
            scanf("%s", book_id_input);
            string book_id = "/api/v1/tema/library/books/" + string(book_id_input);
            free(book_id_input);
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                request = compute_delete_request(local_host, book_id, jwt_token);
            } else {
                request = compute_delete_request(local_host, book_id, jwt_token);
            }
            printf("%s\n", request);
            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            free(request);
            free(response);
        }

        if (strcmp(command, "logout") == 0) {
            // if there are coockies, add them to the request
            if (coockies != NULL && coockies[0][0] != '\0') {
                request = compute_get_request(local_host, "/api/v1/tema/auth/logout", "", coockies, 1, jwt_token);
            } else {
                request = compute_get_request(local_host, "/api/v1/tema/auth/logout", "", NULL, 0, jwt_token);
            }
            printf("%s\n", request);
            send_to_server(sockfd, request);
            response = receive_from_server(sockfd);
            printf("%s\n", response);

            // reset the coockies
            for (int i = 0; i < MAX_COOCKIES; i++) {
                memset(coockies[i], 0, LINELEN);
            }
            printf("%sCoockiesReset%s\n", PRINT_RESPONSE, PRINT_RESPONSE);

            free(request);
            free(response);
        }

        close_connection(sockfd);
    }
    return 0;
    
}

// TODO : Server doesn t respond to any second request
// TODO : Check that page_count is an integer