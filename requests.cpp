#include "requests.hpp"

using namespace std;

string compute_post_request(const char *host, const char *url,
                            const char *content_type, string body_data,
                            const vector<string>& cookies) {
    string message;
    message += "POST " + string(url) + " HTTP/1.1\r\n";
    message += "Host: " + string(host) + "\r\n";
    message += "Content-Type: " + string(content_type) + "\r\n";
    message += "Content-Length: " + to_string(body_data.size()) + "\r\n";

    if (!token_jwt.empty())
        message += "Authorization: Bearer " + token_jwt + "\r\n";

    if (!cookies.empty()) {
        string cookie_header = "Cookie: ";
        for (size_t i = 0; i < cookies.size(); ++i) {
            cookie_header += cookies[i];
            if (i != cookies.size() - 1)
                cookie_header += "; ";
        }
        message += cookie_header + "\r\n";
    }

    message += "\r\n";
    message += body_data;
    return message;
}

string compute_put_request(const char *host, const char *url,
                           const char *content_type, string body_data,
                           string id, const vector<string>& cookies) {
    string message;
    message += "PUT " + string(url) + id + " HTTP/1.1\r\n";
    message += "Host: " + string(host) + "\r\n";
    message += "Content-Type: " + string(content_type) + "\r\n";
    message += "Content-Length: " + to_string(body_data.size()) + "\r\n";

    if (!token_jwt.empty())
        message += "Authorization: Bearer " + token_jwt + "\r\n";

    if (!cookies.empty()) {
        string cookie_header = "Cookie: ";
        for (size_t i = 0; i < cookies.size(); ++i) {
            cookie_header += cookies[i];
            if (i != cookies.size() - 1)
                cookie_header += "; ";
        }
        message += cookie_header + "\r\n";
    }

    message += "\r\n";
    message += body_data;
    return message;
}

string compute_get_request(const char *host, const char *url,
                           const vector<pair<string, string>>& query_params,
                           const vector<string>& cookies) {
    string message;
    if (id.empty())
        message += "GET " + string(url);
    else
        message += "GET " + string(url) + id;
    if (!query_params.empty()) {
         message += "?";
        for (size_t i = 0; i < query_params.size(); i++) {
            message += query_params[i].first + "=" + query_params[i].second;
            if (i != query_params.size() - 1) {
                message += "&";
            }
        }
    }
    message += " HTTP/1.1\r\n";
    message += "Host: " + string(host) + "\r\n";

    if (!cookies.empty()) {
        string cookie_header = "Cookie: ";
        for (size_t i = 0; i < cookies.size(); i++) {
            cookie_header += cookies[i];
            if (i != cookies.size() - 1) {
                cookie_header += "; ";
            }
        }
        message += cookie_header + "\r\n";
    }

    if (!token_jwt.empty())
        message += "Authorization: Bearer " + token_jwt + "\r\n";


    message += "\r\n";
    return message;
}


string compute_delete_request(const char *host, const char *url,
                              const char *username, const vector<string>& cookies) {
    string message;
    message += "DELETE " + string(url) + string(username) + " HTTP/1.1\r\n";
    message += "Host: " + string(host) + "\r\n";

    if (!token_jwt.empty())
        message += "Authorization: Bearer " + token_jwt + "\r\n";

    if (!cookies.empty()) {
        string cookie_header = "Cookie: ";
        for (size_t i = 0; i < cookies.size(); ++i) {
            cookie_header += cookies[i];
            if (i != cookies.size() - 1)
                cookie_header += "; ";
        }
        message += cookie_header + "\r\n";
    }

    message += "\r\n";
    return message;
}
