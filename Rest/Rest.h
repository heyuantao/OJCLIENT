//
// Created by he_yu on 2020-02-07.
//

#ifndef OJCLIENT_REST_H
#define OJCLIENT_REST_H

#include <iostream>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <boost/algorithm/string/replace.hpp>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace std;

//This form data is use for http post and get methods
class HttpFormData{
private:
    std::map<std::string,std::string> data;
public:
    HttpFormData();
    void addItem(std::string key,std::string value);
    std::string toFormString();
    std::map<std::string,std::string> getData();
};

// Http response class
class Response{
public:
    bool status;
    std::string data;
public:
    Response(std::string data,bool status);
};


//use wget to do get or post,the system must have wget
class WgetRest{
public:
    static std::string api_key;                                 //init with ""
public:
    static Response post(std::string api,HttpFormData form);
private:
    static Response readCommandOutput(std::string command);
};

//use miscrosoft Casablanca(cpprestsdk) to get of post
class CasablancaRest{
public:
    //static std::string api_key;                                 //init with ""
    static std::string api_token;
public:
    static Response post(std::string api,HttpFormData form);
};

#endif //OJCLIENT_REST_H
