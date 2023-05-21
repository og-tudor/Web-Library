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
    fgets(username, LINELEN, stdin);
    printf("password=");
    fgets(password, LINELEN, stdin);

    // remove the \n from the end of the strings
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';
    // check if the username and password don't contain spaces
    if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
        free(username);
        free(password);
        credentials.clear();
        return credentials;
    }

    // add the username and password to the credentials
    credentials["username"] = username;
    credentials["password"] = password;

    free(username);
    free(password);
    return credentials;
}

ordered_json read_book_details()
{
    ordered_json book_details;
    char *title = (char *)calloc(LINELEN, sizeof(char));
    char *author = (char *)calloc(LINELEN, sizeof(char));
    char *genre = (char *)calloc(LINELEN, sizeof(char));
    char *page_count = (char *)calloc(LINELEN, sizeof(char));
    char *publisher = (char *)calloc(LINELEN, sizeof(char));
    // read the book details from the user
    printf("title=");
    fgets(title, LINELEN, stdin);
    printf("author=");
    fgets(author, LINELEN, stdin);
    printf("genre=");
    fgets(genre, LINELEN, stdin);
    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    // remove the \n from the end of the strings
    title[strlen(title) - 1] = '\0';
    author[strlen(author) - 1] = '\0';
    genre[strlen(genre) - 1] = '\0';
    publisher[strlen(publisher) - 1] = '\0';
    page_count[strlen(page_count) - 1] = '\0';

    // check if the page_count is a number
    if (strspn(page_count, "0123456789") != strlen(page_count)) {
        free(title);
        free(author);
        free(genre);
        free(publisher);
        free(page_count);
        book_details.clear();
        return book_details;
    }

    // add the book details to the book_details
    book_details["title"] = title;
    book_details["author"] = author;
    book_details["genre"] = genre;
    book_details["publisher"] = publisher;
    book_details["page_count"] = page_count;

    free(title);
    free(author);
    free(genre);
    free(publisher);
    free(page_count);
    return book_details;
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
