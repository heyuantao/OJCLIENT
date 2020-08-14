//
// Created by he_yu on 2020-02-07.
//
#include <iostream>
#include <sstream>
#include <algorithm>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>

#include "Rest.h"
#include "../Common/Exceptions.h"
#include "../Common/Logger.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

HttpFormData::HttpFormData() {
}

void HttpFormData::addItem(std::string key,std::string value) {
    this->data.insert(pair<std::string,std::string>(key,value));
}

std::string HttpFormData::toFormString() {
    std::stringstream ss;
    bool first_pair = true;
    map<std::string,std::string>::iterator iter;
    for(iter=this->data.begin();iter!=this->data.end();iter++){
        if(first_pair== true){
            //final_string=iter->first+"="+iter->second+final_string;
            ss << iter->first << "=" << iter->second ;
            first_pair = false;
        }else{
            //final_string=iter->first+"="+iter->second+"&"+final_string;
            ss << "&" << iter->first << "=" << iter->second ;
        }
    }
    return ss.str();
}

map<std::string, std::string> HttpFormData::getData() {
    return this->data;
}

Response::Response(std::string data,bool status){
    this->data = data;
    this->status = status;
}

string WgetRest::api_key="";

Response WgetRest::readCommandOutput(std::string command){
    std::string final_string ="";
    char buffer[512];
    FILE *file=NULL;

    try{
        file = popen(command.c_str(), "r");
    }catch(...){
        throw ClientRunCommandException("Run Command Error !");
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        final_string = final_string+string(buffer);
    }
    int ret = fclose(file);
    bool status = false;
    if(ret==0){
        return Response(final_string, true);
    }else{
        final_string="";
        return Response(final_string, false);
    }
}

Response WgetRest::post(std::string url,HttpFormData form_data) {
    std::stringstream ss;
    std::string form_data_string = form_data.toFormString();
    //string cmd = "wget --post-data=\""+form_data_string+"\" -q -O - "+"\""+url+"\"";
    ss << boost::format("wget --post-data=\"%s\" -q -O - \"%s\" ") % form_data_string % url << "";
    std::string cmd = ss.str();
    Response res = WgetRest::readCommandOutput(cmd);
    if(res.status== false){
        throw ClientNetworkException(cmd);
    }
    return res;
}

string CasablancaRest::api_token="";

Response CasablancaRest::post(std::string url, HttpFormData form) {
    http_client client(U(url));
    concurrency::streams::stringstreambuf buffer;
    http_request request(methods::POST);
    std::string response_content_string = "";

    try{
        request.headers().add("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
        request.headers().add("Content-Length", "100");  //request.headers().add("Host", "acm01e.syslab.org");
        request.headers().add("X-Requested-With", "XMLHttpRequest");
        request.headers().add("Authorization", CasablancaRest::api_token);
        //request.headers().add("Authorization", "Token " + CasablancaRest::api_token);
        /*request.headers().add("API-KEY",CasablancaRest::api_key);
        if(CasablancaRest::api_key==""){
            ClientLogger::ERROR("Find api_key is emtpy in CasablancaRest::post");
        }*/
        request.set_body(form.toFormString());

        http_response response = client.request(request).get();
        response.body().read_to_end(buffer).get();
        response_content_string = buffer.collection();

        if(response.status_code()==status_codes::OK){
            //status_codes  https://github.com/ambroff/casablanca/blob/d81fd457f877934b00258f4ca4ec079df556f0de/Release/include/cpprest/http_constants.dat
            return Response(response_content_string, true);
        }else if(response.status_code()==status_codes::Unauthorized){
            std::cout<<"Unauthorized in CasablancaRest::post"<<endl;
            throw ClientUnauthorizedException("Unauthorized in CasablancaRest::post");
        }else{
            std::cout<<"Response is empty in CasablancaRest::post"<<endl;
            throw ClientNetworkException("Response is empty in CasablancaRest::post");
        }
    }catch (ClientNetworkException &e) {
        throw e;
    }catch (...){
        std::stringstream ss;
        ss << boost::format("Unknow exception in post data \"%\" with data \"%s\" in CasablancaRest::post") % url % form.toFormString() <<"";
        std::cout<<ss.str()<<endl;
        throw ClientNetworkException(ss.str());
    }

    //return Response(response_content_string, true); //error

    /*
    json::value json_kv ;
    for(map<string,string>::iterator iter=data.begin();iter!=data.end();iter++){
        json_kv[iter->first] = json::value::string(iter->second);
    }
    cout<<json_kv<<endl;
    http_response response = client.request(methods::POST, U("admin/problem_judge.php"), json_kv).get();
    */
    //request.headers().add("good","hello");
    //return response.extract_string().get();
}
