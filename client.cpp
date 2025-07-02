#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "json.hpp"
#include "helpers.h"
#include "requests.hpp"

using namespace std;
using json = nlohmann::json;

string token_jwt = "";
string id="";

void send_http_request(int sockfd, string message) {
    char *buffer =(char *)malloc(message.size() + 1);
    memset(buffer, 0, message.size() + 1);
    strcpy(buffer, message.c_str());
    send_to_server(sockfd, buffer);
    free(buffer);
}

bool valid_id(string id) {
        // verific daca e gol
    if (id.empty()) {
        cout << "ERROR: ID-ul nu poate fi gol." << endl;
        return false;
    }

    //verific fiecare caracter sa vad daca e in intervalul [0,9]
    for (size_t i = 0; i < id.size(); ++i) {
        if (id.c_str()[i] < '0' || id.c_str()[i] > '9') {
            cout << "ERROR: Trebuie sa introduceti un numar valid" << endl;
            return false;
        }
    }
    return true;
}


void login_admin(int sockfd, vector<string>& cookies, bool& admin_connected) {
    string username, password;

    cout << "username=";
    getline(cin, username);

    cout << "password=";
    getline(cin, password);

    //fac verificarile aici, daca nu este ok, nu trimit request-ul
    if (username.find(' ') != string::npos) {
        cout << "ERROR: Username-ul nu trebuie sa contina spatii" << endl;
        return;
    }
    if (password.find(' ') != string::npos) {
        cout << "ERROR: Parola nu trebuie sa contina spatii" << endl;
        return;
    }
    //creez json-ul pentru a putea fi trimis
    json js;
    js["username"] = username;
    js["password"] = password;

    //il transform in string
    string request = js.dump();
    string message = compute_post_request("63.32.125.183", 
            "/api/v1/tema/admin/login", "application/json", request, cookies);    
    send_http_request(sockfd, message);
    char* response = receive_from_server(sockfd);
    //extrag json ul sa vad ce feedback e primit
    char *json_response = basic_extract_json_response(response);
    if (json_response != NULL) {
        //transform in string sa mi fie mai usoara cautarea
        string response_str(json_response);
        if (response_str.find("Admin logged in successfully") != string::npos) {
            cout << "SUCCESS: Admin autentificat cu succes!" << endl;
            admin_connected = true;
            if (strstr(response, "Set-Cookie") != NULL) {
                char *cookie = strstr(response, "Set-Cookie"); //garantat
                char *token = strtok(cookie, " ;");
                token = strtok(NULL, ";");
                string cookie_str(token);
                //daca e prima autentificare
                if (cookies.size() == 0)
                    cookies.push_back(cookie_str);
                else {
                    //poate au fost adaugate cookie-uri
                    cookies.erase(cookies.begin());
                    cookies.push_back(cookie_str);
                }        
            }
        } else if (response_str.find("Already logged in") != string::npos) {
            cout << "ERROR: Admin deja logat!" << endl;
        } else if (response_str.find("Invalid credentials")  != string::npos) {
            cout << "ERROR: Credentiale gresite!" << endl;
        } else {
            cout << "ERROR: Raspuns necunoscut:" << response_str << endl;
        }
    }
}

void login(int sockfd, vector<string>& cookies, bool& user_connected) {
    string admin_username, username, password;
    cout << "admin_username=";
    getline(cin, admin_username);

    cout << "username=";
    getline(cin, username);

    cout << "password=";
    getline(cin, password);

    //tratez input-ul invalid

    if (username == "" || password == "") {
        cout << "ERROR: Introduceti credentiale valide" << endl;
        return;
    }

    if (username.find(' ') != string::npos) {
        cout << "ERROR: Username-ul nu trebuie sa contina spatii" << endl;
        return;
    }
    if (password.find(' ') != string::npos) {
        cout << "ERROR: Parola nu trebuie sa contina spatii" << endl;
        return;
    }

    //completez json-ul
    json js;
    js["admin_username"] = admin_username;
    js["username"] = username;
    js["password"] = password;

    string request = js.dump();
    //compun mesajul
    string message = compute_post_request("63.32.125.183",
            "/api/v1/tema/user/login", "application/json", request, cookies);
    //trimit request-ul
    send_http_request(sockfd, message);
    //astept raspunsul
    char* response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("User logged in successfully") != string::npos) {
            cout << "SUCCESS: Te-ai autentificat ca user." << endl;
            //daca s-a conectat cu succes, trebuie sa pun in cookies sesiunea
            if (strstr(response, "Set-Cookie") != NULL) {
                //garantat, altfel nu facea match strstr-ul
                char *cookie = strstr(response, "Set-Cookie");
                char *token = strtok(cookie, " ;");
                token = strtok(NULL, ";");
                string cookie_str(token);
                cookies.push_back(cookie_str);
                user_connected = true;
            }
        } else if (json_res.find("Invalid credentials")  != string::npos) {
            cout << "ERROR: Creditentiale gresite." << endl;
        } else if (json_res.find("Admin user not found") != string::npos)
            cout << "ERROR: Admin user nu e corect" << endl;
    }
}

void add_user(int sockfd, vector<string>&cookies) {
    string username, password;
    cout << "username=";
    getline(cin, username);

    cout << "password=";
    getline(cin, password);

    if (username == "" || password == "") {
        cout << "ERROR: Introduceti credentiale valide" << endl;
        return;
    }
    
    if (username.find(' ') != string::npos || password.find(' ') != string::npos) {
        cout << "ERROR: Numele de utilizator si parola nu trebuie sa contina spatii" << endl;
        return;
    }

    //dupa verificiari, pun in json
    json js;
    js["username"] = username;
    js["password"] = password;

    string request = js.dump();
    //trimit request-ul
    string message = compute_post_request("63.32.125.183",
                "/api/v1/tema/admin/users", "application/json", 
                request, cookies);   
    send_http_request(sockfd, message);
    char* response = receive_from_server(sockfd);
    //daca am primit ceva
    if (response != NULL) {
        //fac match
        string json_res(basic_extract_json_response(response));
        if (json_res.find("User created successfully") != string::npos)
            cout << "SUCCESS: User-ul a fost adaugat." << endl;
        else if (json_res.find("User already exists") != string::npos)
            cout << "ERROR: User-ul deja exista." << endl;
        else if (json_res.find("Admin privileges required") != string::npos)
            cout << "ERROR: Nu aveti drepturi de admin." << endl;
        else 
            cout << "ERROR: Alta eroare a aparut" << endl;
    }
}

void get_users(int sockfd, const vector<string>&cookies) {
    vector<pair<string, string>> params; // gol
    string message = compute_get_request("63.32.125.183", 
        "/api/v1/tema/admin/users", params, cookies);
    send_http_request(sockfd, message);
    char* response = receive_from_server(sockfd);
    if (response != NULL) {
        // ori nu exista acces
        string json_res(basic_extract_json_response(response));
        if (json_res.find("Admin privileges required") != string::npos) {
            cout << "ERROR: Admin privileges required" << endl;
            return;
        }
        //ori exista si iterez prin json-ul primit
        json js = json::parse(json_res);
        for (const auto& user : js["users"]) {
            string incorrect_username =  user["username"];
            string username = incorrect_username.substr(0, incorrect_username.size());
            string incorrect_password = user["password"];
            string password = incorrect_password.substr(0, incorrect_password.size());
            cout << "#" << user["id"] << " " << username <<":" << password << endl;
        }
    }
}

void delete_user(int sockfd, const vector<string>& cookies) {
    string username;
    cout << "username=";
    getline(cin, username);
    //daca este gol
    if (username.empty()) {
        cout << "ERROR: Niciun username" << endl;
        return;
    }
    string message = compute_delete_request("63.32.125.183", "/api/v1/tema/admin/users/", 
                                            username.c_str(), cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
            char *json_ptr = basic_extract_json_response(response);
        //daca nu primesc nimic in json(am patit)
        if (json_ptr == NULL) {
            cout << "ERROR: JSON invalid in raspuns." << endl;
            free(response); 
            return;
        }
        string json_res(json_ptr);
        if (json_res.find("User not found") != string::npos)
            cout << "ERROR: User-ul nu a fost gasit/" << endl;
        else if (json_res.find("deleted successfully")  != string::npos)
            cout << "SUCCESS: User-ul a fost sters." << endl;
        else if (json_res.find("Admin privileges required")  != string::npos)
            cout << "ERROR: Nu aveti drepturi de admin." << endl;
        else 
            cout << "ERROR: Alta eroare." << endl;
    }
}

void get_access(int sockfd, vector<string>& cookies) {
    vector<pair<string, string>> query;
    string message = compute_get_request("63.32.125.183", "/api/v1/tema/library/access", 
                                        query, cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("Authentication required") != string::npos) {
            cout << "ERROR: Nu sunteti logat." << endl;
        //caut cuvantul token pentru a sti sigur ca exista in raspuns
        } else if(json_res.find("token") != string::npos) {
            //iau un pointer la inceputul json-ului
            char *payload = strstr(response, "token"); 
            char *token = strtok(payload, ":"); // escape
            token = strtok(NULL, "\""); // token ul
            //teoretic mereu e nenull 
            if (token != NULL) 
                token_jwt = token;
        cout << "SUCCESS: Token JWT primit." << endl;
        }
    }
}

void get_movies(int sockfd, const vector<string>& cookies) {
    //vector care poate fi folosit, in cazul asta este gol
    vector<pair<string, string>> query;
    string message = compute_get_request("63.32.125.183", "/api/v1/tema/library/movies", 
                                        query, cookies); //compun mesajul
    send_http_request(sockfd, message); //trimit mesajul
    char *response = receive_from_server(sockfd); // astept raspuns
    if (response != NULL) { //daca am primit mesaj, fac match pe ce raspuns am primit
        string json_res(basic_extract_json_response(response));
        if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: Trebuie sa luati JWT token" << endl;
        else if (json_res.find("Current user not found") != string::npos) {
            cout << "ERROR: Current user not found" << endl;
        } else if (json_res.find("movies") != string::npos) {
            //intrucat e un vector, am lucrat cu functiile oferite de biblioteca json
            json js = json::parse(json_res);
            //daca nu exist afilme
            if (js["movies"].empty())
                cout << "SUCCESS: Nu exista filme";
            else {
                //cel putin un element
                cout << "SUCCESS: Lista filmelor" << endl;
                //iterez prin toate filmele
                for (auto& movie : js["movies"]) {
                    //era afisat cu "test", asa ca am facut escape primului
                    //si ultimului caracter prin substr.
                    string fake_title = movie["title"];
                    string title = fake_title.substr(0, fake_title.size());
                    cout << "#" << movie["id"] << endl;
                    cout << title << endl;
                }
            }
        }
    }
}

void get_movie(int sockfd, const vector<string>& cookies) {
    cout << "id=";
    //citesc direct in variabila globala
    getline(cin, id);

    if (valid_id(id) == false)
        return;

    // useless pentru impelemntarea cureneta
    vector<pair<string, string>> query;
    string message = compute_get_request("63.32.125.183", 
                    "/api/v1/tema/library/movies/", query, cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("Movie not found") != std::string::npos)
            cout << "ERROR: Nu s-a gasit filmul" << endl;
        else if (strstr(response, "token") != NULL) {
            cout << "ERROR: JWT token necesar." << endl;
        }
        else if (strstr(response, "OK") != NULL) {
            //intrucat lucrez cu mai multe campuri,
            //am folosit implementarea din json,
            //facand escape primului si ultimului caracter
            //din string
            json js = json::parse(json_res);
            string fake_title = js["title"];
            string fake_description = js["description"];
            string title = fake_title.substr(0, fake_title.size());
            string description = fake_description.substr(0, fake_description.size());
            string fake_rating = js["rating"];
            string rating = fake_rating.substr(0, fake_rating.size());
            cout << "title: " << title << endl;
            cout << "year: " << js["year"] << endl;
            cout << "description: " << description << endl;
            cout << "rating: " << rating << endl;
        }
    }
    //sterg id-ul, in caz de se pune in alt request
    id = "";
}

void logout_admin(int sockfd, vector<string>& cookies, bool& admin_connected) {
    vector<pair<string, string>> query;
    string message = compute_get_request("63.32.125.183",
                     "/api/v1/tema/admin/logout", query,
                    cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("Admin logged out successfully") != string::npos) {
            cout << "SUCCESS: Adminul s-a deconectat." << endl;
            admin_connected = false;
            //sterg coockie-urile pentru ca s-a terminat sesiunea
            cookies.erase(cookies.begin());
        }
        else if (json_res.find("Admin privileges required") != string::npos)
            cout << "ERROR: Nu sunteti autentificat ca admin.";
    }
    //sterg token-ul
    token_jwt = "";
}

void add_movie(int sockfd, const vector<string>& cookies) {
    string title, description;
    int year;
    double rating;
    bool valid = true;
    cout << "title=";
    getline(cin, title);
    if (title.empty())
        valid = false;

    cout << "year=";
    cin >> year;
    if (cin.fail() || year < 1800 || year > 2025)
        valid = false;
    cin.ignore(); // pentru \n

    cout << "description=";
    getline(cin, description);

    cout << "rating=";
    cin >> rating;

    //tratez ca input ul sa fie valid
    if (valid == false) {
        cout << "ERROR: Nu ati introdus date corecte.";
        return;
    }
    //creez json-ul
    json js;
    js["title"] = title;
    js["year"] = year;
    js["description"] = description;
    js["rating"] = rating;

    string payload = js.dump();
    string message = compute_post_request("63.32.125.183", 
                    "/api/v1/tema/library/movies", "application/json",
                    payload, cookies);

    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        //mesajele care pot aparea in response, respectiv in json ul din response
        string json_res(basic_extract_json_response(response));
        if (string(response).find("CREATED") != string::npos)
            cout << "SUCCESS: Filmul a fost adaugat" << endl;
        else if(json_res.find("Rating must be between 0 and 9.9") != string::npos)
            cout << "ERROR: Rating invalid" << endl;
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token invalid";
    }
}

void delete_movie(int sockfd, const vector<string>& cookies) {
    cout <<"id=";
    string str;
    getline(cin, str);

    if (valid_id(str) == false)
        return;

    //compun mesajul
    string message = compute_delete_request("63.32.125.183", 
                                        "/api/v1/tema/library/movies/",
                                        str.c_str(), cookies);
    //trimit mesajul
    send_http_request(sockfd, message);
    //astept mesaj de la server
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        //tratez raspunsurile venite de la server
        string json_res(basic_extract_json_response(response));
        if (json_res.find("Invalid movie id") != string::npos)
            cout << "ERROR: Id-ul nu este valid." << endl;
        else if (json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token necesar." << endl;
        else if (json_res.find("Movie with id " + str + " deleted successfully") != string::npos)
            cout << "SUCCES: Filmul a fost sters." << endl;
    }
}

void update_movie(int sockfd, const vector<string>& cookies) {
    int fake_id, year;
    string title, description;
    float rating;

    cout << "id=";
    cin >> fake_id;
    cin.ignore();
    
    string id = to_string(fake_id);

    cout << "title=";
    getline(cin, title);

    cout << "year=";
    cin >> year;
    // elimin /n de la final
    cin.ignore();

    cout << "description=";
    getline(cin, description);

    cout << "rating=";
    cin >> rating;

    json js;
    js["id"] = id;
    js["title"] = title;
    js["year"] = year;
    js["description"] = description;
    js["rating"] = rating;

    string request = js.dump();
    string message = compute_put_request("63.32.125.183",
                    "/api/v1/tema/library/movies/", 
                    "application/json", request, id, cookies);
    
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (strstr(response, "OK") != NULL)
            cout << "SUCCESS: Filmul a fost updatat.";
        else if (json_res.find("Current user not found") != string::npos)
            cout << "ERROR: User-ul nu a fost gasit." << endl;
        else if(json_res.find("Invalid movie id") != string::npos)
            cout << "ERROR: Invalid id." << endl;
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token necesar.";
    }
}

void get_collections(int sockfd, const vector<string>& cookies) {
    vector<pair<string, string>> query;
    string message = compute_get_request("63.32.125.183", 
                    "/api/v1/tema/library/collections", query, cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (string(response).find("OK") != string::npos) {
            json js = json::parse(json_res);
            if (js["collections"].empty())
                cout << "SUCCESS: Niciun collections" << endl;
            else {
                cout << "SUCCESS: Colectiile sunt" << endl;
                for (auto& collection : js["collections"]) {
                    string fake_title = collection["title"];
                    string title = fake_title.substr(0, fake_title.size());
                    cout << "#" << collection["id"] << ": " << title << endl;
                }
            }
        }
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token necesar.";
    }
} 

void get_collection(int sockfd, const vector<string>& cookies) {
    cout <<"id=";
    getline(cin, id);

    if (valid_id(id) == false)
        return;

    vector <pair<string, string>> query;
    string message = compute_get_request("63.32.125.183", 
                    "/api/v1/tema/library/collections/", query, cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (string(response).find("OK") != string::npos) {
            json js = json::parse(json_res);
            string title = js["title"];
            string owner = js["owner"];

            cout <<"SUCCESS: Detalii colectie" << endl;
            cout << "title: " << title.substr(0, title.size()) << endl;
            cout << "owner: " << owner.substr(0, owner.size()) << endl;
            for (auto& movie : js["movies"]) {
                string movie_name = movie["title"];
                cout << "#" << movie["id"] << ": " << movie_name.substr(0, movie_name.size()) << endl;  
            }
        }
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token";
        else if(json_res.find("Invalid collection id or you do not own this collection") != string::npos)
            cout << "ERROR: Id-ul colectiei este invalid sau nu detineti colectia.";
    }
    id = "";
}

bool add_movie_to_collection(int sockfd, const vector<string>& cookies, 
                            bool from_user, int movie, string collection) {
    string collection_id;
    int movie_id;
    //comanda provenita din main
    if (from_user) {
        cout << "collection_id=";
        getline(cin, collection_id);
        cout << "movie_id=";
        cin >> movie_id;
    //comanda provenita din add_collection
    } else {
        collection_id = collection;
        movie_id = movie;
    }

    if (valid_id(collection_id) == false || valid_id(to_string(movie_id)) == false)
        return false;

    string url = "/api/v1/tema/library/collections/" + collection_id + "/movies";
    json js;
    js["id"] = movie_id;
    string message = compute_post_request("63.32.125.183",
                        url.c_str(), "application/json", js.dump(), cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("Movie added to collection successfully") != string::npos) { //
            cout << "SUCCESS: Adaugat corect" << endl;
            return true;
        }
        else if (json_res.find("Invalid collection id or you do not own this collection") != string::npos) {
            cout << "ERROR: Colectie invalida sau nu detineti aceasta colectie." << endl;
            return false;
        }
        else if(json_res.find("JWT token required") != string::npos) {
            cout << "ERROR: JWT token necesar." << endl;
            return false;
        }
    }
    return false;
}

void add_collection(int sockfd, const vector<string>& cookies) {
    string title;
    int num_movies;

    cout <<"title=";
    getline(cin, title);

    cout << "num_movies=";
    cin >> num_movies;

    //crez json-ul doar cu titlul in prima faza
    json js;
    js["title"] = title;
    string message = compute_post_request("63.32.125.183", 
                    "/api/v1/tema/library/collections", "application/json",
                    js.dump(), cookies);

    //trimit sa vad daca se poate face colectia
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    bool succes = true;
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        //daca s-a creat
        if (string(response).find("201 CREATED") != string::npos) {
            int movie_id;
            //escape pana la id-ul colectiei
            char *token = basic_extract_json_response(response);
            char *tok = strtok(token, ":");
            tok = strtok(NULL, ","); //am extras id ul
            string collection_id = string(tok);
            //adaug cele num_movies filme
            for (int i = 0; i < num_movies; ++i) {
                cout << "movie_id[" << i << "]=";
                cin >> movie_id;
                bool ret = add_movie_to_collection(sockfd, cookies, false, movie_id, collection_id);
                //daca cel putin unul a esuat, esueaza si adaugarea colectiei
                if (ret == false)
                    succes = false;
            }
            if (succes == true)
                cout << "SUCCESS: Colectie adaugata cu succes" << endl;
            else
                cout << "ERROR: Nu ati introdus toate id-urile filemlor corect" << endl;
        }
        else if (string(response).find("400 BAD REQUEST") != string::npos)
            cout << "ERROR: Trebuie sa fie un string" << endl;
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token necesar." << endl;
    }
}

void delete_collection(int sockfd, const vector<string>& cookies) {
    cout << "id=";
    getline(cin, id);

    if (valid_id(id) == false)
        return;

    string message = compute_delete_request("63.32.125.183",
                    "/api/v1/tema/library/collections/", id.c_str(),
                    cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (string(response).find("201 CREATED") != string::npos)
            cout << "SUCCESS: Adaugat corect." << endl;
        else if (string(response).find("Invalid collection id or you do not own this collection") != string::npos)
            cout << "ERROR: Id-ul colectiei este invalid sau nu detineti aceasta colectie." << endl;
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token necesar." << endl;
    }
    //resetez id-ul, fiind variabila globala, sa nu am neplaceri la cererile viitoare
    id ="";
}


void delete_movie_from_collecetion(int sockfd, const vector<string>& cookies) {
    string collection_id, movie_id;
    cout << "collection_id=";
    getline(cin, collection_id);
    cout << "movie_id=";
    getline(cin, movie_id);

    if (valid_id(collection_id) == false || valid_id(movie_id) == false)
        return;
    //am creat url-ul manual, era mai complicat sa-l creez in 
    //"compute_delete_request"
    string url = "/api/v1/tema/library/collections/" + collection_id 
                + "/movies/";
    string message = compute_delete_request("63.32.125.183", 
                    url.c_str(), movie_id.c_str(), cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("removed from collection") != string::npos)
            cout << "SUCCESS: Eliminat corect" << endl;
        else if (json_res.find("Invalid collection id or you do not own this collection") != string::npos)
            cout << "ERROR: Id-ul colectiei este invalid sau nu detineti aceasta colectie." << endl;
        else if(json_res.find("JWT token required") != string::npos)
            cout << "ERROR: JWT token necesar." << endl;
    }
    
}

void logout(int sockfd, vector<string>& cookies, bool& user_connection) {
    vector<pair<string, string>> query;
    //trimit cererea la URL ul respectiv
    string message = compute_get_request("63.32.125.183",
                     "/api/v1/tema/user/logout", query,
                    cookies);
    send_http_request(sockfd, message);
    char *response = receive_from_server(sockfd);
    
    if (response != NULL) {
        string json_res(basic_extract_json_response(response));
        if (json_res.find("User logged out successfully") != string::npos) {
            cout << "SUCCESS: Utilizatorul a fost delogat." << endl;
            user_connection = false;
            //sterg cookie-ul de sesiune
            cookies.clear();
        }
        else if (json_res.find("Authentication required") != string::npos)
            cout << "ERROR: Nu sunteti autentificat.";
    }
    //sterg token-ul
    token_jwt = "";
}

int main() {
    char ip[14];
    strcpy(ip, "63.32.125.183");
    vector<string> cookies;
    //clientul ruleaza in loop
    bool admin_connected = false, user_connected = false;
    while (1) {
        //la fiecare comanda se deschide un nou canal TCP
        int sockfd = open_connection(ip, 8081, AF_INET, SOCK_STREAM, 0);
        string command, empty;
        getline(cin, command);

        if(command == "login") {
            if (!user_connected && !admin_connected)
                login(sockfd, cookies, user_connected);
            else
                cout << "ERROR: Sunteti deja conectat." << endl;
        }
    
        if (command == "login_admin") {
            if (!user_connected && !admin_connected)
                login_admin(sockfd, cookies, admin_connected);
            else
                cout << "ERROR: Esti deja conectat." << endl;
        }
        if (command == "add_user") {
                add_user(sockfd, cookies);
        }

        if (command == "get_users")
            get_users(sockfd, cookies);

        if (command == "delete_user")
            delete_user(sockfd, cookies);


        if (command == "get_access")
            get_access(sockfd, cookies);

        if (command == "get_movies")
            get_movies(sockfd, cookies);

        if (command == "get_movie")
            get_movie(sockfd, cookies);

        if (command == "logout_admin")
            logout_admin(sockfd, cookies, admin_connected);

        if (command == "add_movie")
            add_movie(sockfd, cookies);

        if (command == "delete_movie")
            delete_movie(sockfd, cookies);

        if (command == "update_movie")
            update_movie(sockfd, cookies);
        
        if (command == "get_collections")
            get_collections(sockfd, cookies);

        if (command == "get_collection")
            get_collection(sockfd, cookies);

        if (command == "add_collection")
            add_collection(sockfd, cookies);

        if (command == "delete_collection")
            delete_collection(sockfd, cookies);
        
        if (command == "add_movie_to_collection")
            add_movie_to_collection(sockfd, cookies, true, 0, empty);

        if (command == "delete_movie_from_collection")
            delete_movie_from_collecetion(sockfd, cookies);

        if (command == "logout")
            logout(sockfd, cookies, user_connected);

        if (command == "exit") {
            close_connection(sockfd);
            break;
        }

        //dupa fiecare comanda inchid canalul respectiv
        close_connection(sockfd);
    }
}