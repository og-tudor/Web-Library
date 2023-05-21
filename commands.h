#ifndef _COMMANDS_
#define _COMMANDS_

#include <string>
using namespace std;
#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

// Register a new user
void register_user(int sockfd, string local_host, string url,
                   string content_type, ordered_json *jwt_token);

// Login an existing user
void login_user(int sockfd, string local_host, string url,
                string content_type, ordered_json *jwt_token, char **coockies);

// Returns the JWT token to acces the library, if the user is logged in
ordered_json enter_library(int sockfd, string local_host, 
                           string url, ordered_json *jwt_token, char **coockies);

// Returns the list of books, if the user is logged in
void get_library(int sockfd, string local_host, string url,
               ordered_json *jwt_token, char **coockies);

// Returns the details of a book based on id, if the user is logged in
void get_book(int sockfd, string local_host, string url,
              ordered_json *jwt_token, char **coockies);

// Adds a new book to the library, if the user is logged in
void add_book(int sockfd, string local_host, string url,
              string content_type, ordered_json *jwt_token, char **coockies);

// Deletes a book from the library, if the user is logged in
void delete_book(int sockfd, string local_host, string url,
                 ordered_json *jwt_token, char **coockies);

// Logout the user, if the user is logged in
void logout_user(int sockfd, string local_host, string url,
            ordered_json *jwt_token, char **coockies);

#endif