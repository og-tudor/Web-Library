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

int main() {
    string local_host = "34.254.242.81:8080";
    // get just the host_ip from local_host
    string host_ip = local_host.substr(0, local_host.find(":"));
    string x_form = "application/x-www-form-urlencoded";
    string json_form = "application/json";
    string book_id;
    char *command = (char *)calloc(100, sizeof(char));
    char *garbage = (char *)calloc(100, sizeof(char));
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
        // read the command from the user
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
            register_user(sockfd, local_host, "/api/v1/tema/auth/register",
                          json_form, &jwt_token);
        }

        if (strcmp(command, "login") == 0) {
            login_user(sockfd, local_host, "/api/v1/tema/auth/login", 
                       json_form, &jwt_token, coockies);
        }

        if (strcmp(command, "enter_library") == 0) {
            jwt_token = enter_library(sockfd, local_host,
                                      "/api/v1/tema/library/access", &jwt_token, coockies);
        }

        if (strcmp(command, "get_books") == 0) {
            get_library(sockfd, local_host, "/api/v1/tema/library/books", 
                        &jwt_token, coockies);
        }

        if (strcmp(command, "get_book") == 0) {
            get_book(sockfd, local_host, "/api/v1/tema/library/books", 
                     &jwt_token, coockies);
        }

        if (strcmp(command, "add_book") == 0) {
            add_book(sockfd, local_host, "/api/v1/tema/library/books", 
                     json_form, &jwt_token, coockies);
        }

        if (strcmp(command, "delete_book") == 0) {
            delete_book(sockfd, local_host, "/api/v1/tema/library/books", 
                        &jwt_token, coockies);
        }

        if (strcmp(command, "logout") == 0) {
            logout_user(sockfd, local_host, "/api/v1/tema/auth/logout", 
                        &jwt_token, coockies);
        }

        close_connection(sockfd);
    }
    return 0;
    
}
