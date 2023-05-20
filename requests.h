#ifndef _REQUESTS_
#define _REQUESTS_

#include <string>
using namespace std;
#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;


// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(string host, string url, string query_params,
							char **cookies, int cookies_count, ordered_json jwt_token);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(string host, string url, string content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count);

char *compute_json_post_request(string host, string url, string content_type,
								ordered_json body_data, char** cookies,
								int cookies_count, ordered_json jwt_token);

char *compute_delete_request(string host, string url, ordered_json jwt_token);


#endif