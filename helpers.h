#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000
#define MAX_COOCKIES 5

#include <iostream>
#include <nlohmann/json.hpp>

using ordered_json = nlohmann::ordered_json;
using namespace std;

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(string host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// receives input from the user and returns the credentials in a JSON format
ordered_json read_user_credentials();

// receives input from the user and returns the book in a JSON format
ordered_json read_book_details();

// receives input from the server and extracts the coockies
void extract_coockies(char *response, char **coockies);

#endif
