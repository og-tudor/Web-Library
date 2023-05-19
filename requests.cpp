#include "requests.h"
#include "helpers.h"
#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */
#include <string>
#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;


using namespace std;

char *compute_get_request(string host, string url, string query_params,
                          char **cookies, int cookies_count, ordered_json jwt_token) {
  char *message = (char*)calloc(BUFLEN, sizeof(char));
  char *line = (char*)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL, request params (if any) and protocol
  // type
  if (query_params.data() != NULL) {
    sprintf(line, "GET %s?%s HTTP/1.1", url.data(), query_params.data());
  } else {
    sprintf(line, "GET %s HTTP/1.1", url.data());
  }

  compute_message(message, line);

  // Step 2: add the host
  sprintf(line, "Host: %s", host.data());
  compute_message(message, line);

  // add headers and/or cookies, according to the protocol
  // Add the jwt token if it exists
  if (!jwt_token.empty()) {
    memset(line, 0, LINELEN);
    string token;
    token = jwt_token["token"];
    sprintf(line, "Authorization: Bearer %s", token.data());
    compute_message(message, line);
  }
  // adding cookies if any are present
  if (cookies != NULL) {
    memset(line, 0, LINELEN);
    strcat(line, "Cookie: ");
    for (int i = 0; i < cookies_count - 1; i++) {
      strcat(line, cookies[i]);
      strcat(line, "; ");
    }

    strcat(line, cookies[cookies_count - 1]);
    compute_message(message, line);
  }

  // Step 4: add final new line
  compute_message(message, "");
  free(line);
  return message;
}

char *compute_post_request(string host, string url, string content_type,
                           char **body_data, int body_data_fields_count,
                           char **cookies, int cookies_count) {
  char *message = (char*)calloc(BUFLEN, sizeof(char));
  char *line = (char*)calloc(LINELEN, sizeof(char));
  char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL and protocol type
  sprintf(line, "POST %s HTTP/1.1", url.data());
  compute_message(message, line);

  // Step 2: add the host
  sprintf(line, "Host: %s", host.data());
  compute_message(message, line);

  /* Step 3: add necessary headers (Content-Type and Content-Length are
     mandatory) in order to write Content-Length you must first compute the
     message size
  */
 
  memset(body_data_buffer, 0, LINELEN);
  for (int i = 0; i < body_data_fields_count; ++i) {
    strcat(body_data_buffer, body_data[i]);
    if (i != body_data_fields_count - 1) {
      strcat(body_data_buffer, "&");
    }
  }

  sprintf(line, "Content-Type: %s", content_type.data());
  compute_message(message, line);
  
  sprintf(line, "Content-Length: %lu", strlen(body_data_buffer));
  compute_message(message, line);



  // Step 4 (optional): add cookies
  if (cookies != NULL) {
    memset(line, 0, LINELEN);
    strcat(line, "Cookie: ");
    for (int i = 0; i < cookies_count - 1; i++) {
      strcat(line, cookies[i]);
      strcat(line, ";");
    }

    strcat(line, cookies[cookies_count - 1]);
    compute_message(message, line);
  }

  // Step 5: add new line at end of header
  compute_message(message, "");

  // Step 6: add the actual payload data
  memset(line, 0, LINELEN);
  strcat(message, body_data_buffer);

  free(line);
  free(body_data_buffer);
  return message;
}


char *compute_json_post_request(string host, string url, string content_type,
                                ordered_json body_data, char **cookies,
                                int cookies_count, ordered_json jwt_token) {
  char *message = (char*)calloc(BUFLEN, sizeof(char));
  char *line = (char*)calloc(LINELEN, sizeof(char));
  char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));
  // write the method name, URL and protocol type
  sprintf(line, "POST %s HTTP/1.1", url.data());
  compute_message(message, line);

  // add the host
  sprintf(line, "Host: %s", host.data());
  compute_message(message, line);

  // Add necessary headers Content Type and Content Length for json format
  sprintf(line, "Content-Type: %s", content_type.data());
  compute_message(message, line);
  sprintf(line, "Content-Length: %lu", body_data.dump().size());
  compute_message(message, line);
  
  // add jwt token if it exists
  if (!jwt_token.empty()) {
    memset(line, 0, LINELEN);
    string token;
    token = jwt_token["token"];
    sprintf(line, "Authorization: Bearer %s", token.data());
    compute_message(message, line);
  }
  // add cookies
  if (cookies != NULL) {
    memset(line, 0, LINELEN);
    strcat(line, "Cookie: ");
    for (int i = 0; i < cookies_count - 1; i++) {
      strcat(line, cookies[i]);
      strcat(line, ";");
    }

    strcat(line, cookies[cookies_count - 1]);
    compute_message(message, line);
  }
  // add new line at end of header
  compute_message(message, "");

  // add the actual payload data
  memset(line, 0, LINELEN);
  strcat(message, body_data.dump().c_str());

  free(line);
  free(body_data_buffer);
  return message;
}
