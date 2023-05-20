#ifndef _COMMANDS_
#define _COMMANDS_

#include <string>
using namespace std;
#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

// Register a new user
void register_user(int sockfd, string local_host, string url, string content_type, ordered_json jwt_token);

// Login an existing user
void login_user(int sockfd, string local_host, string url, string content_type, ordered_json jwt_token, char **coockies);

// Returns the JWT token to acces the library, if the user is logged in
ordered_json enter_library(int sockfd, string local_host, string url, ordered_json jwt_token, char **coockies);

// Returns the list of books, if the user is logged in
void get_books(int sockfd, string local_host, string url, ordered_json jwt_token, char **coockies);

#endif