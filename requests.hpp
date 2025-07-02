#pragma once

#include <string>
#include <vector>

using namespace std;

extern string id;
extern string token_jwt;

string compute_post_request(const char *host, const char *url,
                            const char *content_type, string body_data,
                            const vector<string>& cookies);

string compute_put_request(const char *host, const char *url,
                           const char *content_type, string body_data,
                           string id, const vector<string>& cookies);

string compute_get_request(const char *host, const char *url,
                           const vector<pair<string, string>>& query_params,
                           const vector<string>& cookies);

string compute_delete_request(const char *host, const char *url,
                              const char *username, const vector<string>& cookies);
