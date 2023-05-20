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
    // read the username and password from the user
    char* request;
    char* response;
    ordered_json credentials;
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