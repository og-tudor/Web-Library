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
#include "helpers.h"
#include "buffer.h"
#include <nlohmann/json.hpp>

using namespace std;
using ordered_json = nlohmann::ordered_json;

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(string host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    // convert the string host_ip to a network address in binary form
    inet_aton(host_ip.data(), &serv_addr.sin_addr);
    // inet_pton(AF_INET, "34.254.242.81:8080", &(serv_addr.sin_addr));

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");


    return sockfd;
}

ordered_json read_user_credentials()
{
    ordered_json credentials;
    char *username = (char *)calloc(LINELEN, sizeof(char));
    char *password = (char *)calloc(LINELEN, sizeof(char));
    // read the username and password from the user
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);
    // add the username and password to the credentials
    credentials["username"] = username;
    credentials["password"] = password;

    free(username);
    free(password);
    return credentials;
}

void extract_coockies(char *response, char** coockies)
{
    // char *coockie = (char *)calloc(LINELEN, sizeof(char));
    char* coockie;
    coockie = strstr(response, "Set-Cookie:");
    if (coockie == NULL) {
        return;
    }

    coockie += strlen("Set-Cookie:");
    
    // delete the ; from the end of the coockie
    char *end = strstr(coockie, ";");

    // get first cookie if not null
    if (end != NULL) {
        strncpy(coockies[0], coockie, end - coockie);
        coockies[0][end - coockie] = '\0';
    } else {
        strcpy(coockies[0], coockie);
    }
    

    // get the rest of the cookies
    int i = 1;
    while (coockie != NULL && i < MAX_COOCKIES) {
        coockie = strstr(coockie, "Set-Cookie:");
        if (coockie == NULL) {
            break;
        }
        coockie += strlen("Set-Cookie:");
        sscanf(coockie, "%s", coockies[i]);
        i++;
    }
    free(coockie);
    // print the coockies
    return;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}
