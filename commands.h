#ifndef _COMMANDS_
#define _COMMANDS_

#include <string>
using namespace std;
#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

// Register a new user
void register_user(int sockfd, string local_host, string url, string content_type, ordered_json jwt_token);

#endif