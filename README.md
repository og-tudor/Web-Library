*Frecus Tudor George - 324CA*
# Web-Client_Rest-Api
###
## Description
To understand the fundamentals of REST architecture, we will build a web client in C++ that
interacts with a REST API exposed by a server via the HTTP protocol.
The server exposes a REST (Representational
State Transfer).      
 You can think of it as a black box that has a series of exposed inputs,  
represented by HTTP routes. Following HTTP requests, the server performs an action.     
In the context of
theme, the server simulates an online library and is already fully implemented.     
(HOST : 34.254.242.81 PORT : 8080)  
The client is a program written in C/C++ that accepts commands from the keyboard and sends, according to
requests to the server. Its purpose is to function as a command line interface
(CLI) with the virtual library.

## Interaction with the server
The client will interact with the server via HTTP requests.  
The folowing HTTP requests were implemented:
* compute_json_post_request - sends a POST request to the server with a JSON body
* compute_get_request - sends a GET request to the server
* compute_delete_request - sends a DELETE request to the server

The client will send HTTP requests to the server and will receive HTTP responses from the server.
The client will interact with the terminal using the following commands:
* register - sends a POST request to the server with the username and password
* login - sends a POST request to the server with the username and password   
and stores the session cookie in the main program
* enter_library - sends a GET request to the server and  
 receives a JWT token that will be used for the next requests
* get_books - sends a GET request to the server and receives a list of books
* get_book - sends a GET request to the server and receives a book
* add_book - sends a POST request to the server with a book
* delete_book - sends a DELETE request to the server with a book
* logout - sends a GET request to the server and deletes the token

## Implementation
A request is sent using the appropriate function for each command,  
the response is received and based on the response, the appropriate message is displayed.   
(200 - OK - "Request successful" || 400 - Bad Request - "Request failed")  

The send_to_server(), receive_from_server(), open_connection() and close_connection() functions,  
 are taken from the laboratory.

For each command, the appropriate function will check if the response is valid or not,  
besides login and register, all functions will check if the user is logged in,  
and besides enter_library, logout and the 2 other mentioned above,   
all functions will check if the user has a JWT token.  
For reading the credeentials and book details, I made two functions in helper.cpp.    
The functions are read_credentials() and read_book_details(), which read input  
 and check if it is valid.
All functions that require an id will check if the id is valid.  

If any of the checks fail, the function will return an error message.

## Personal comments
* I found this homework very interesting and helpful for understanding  
 the REST API and the HTTP protocol.
* I had some problems with the connection to the server, the socket would expire after a while,  
so I had to close and reopen the connection.
* I found the difficulty of the homework to be perfect,  
challenging but not too hard as to be impossible.