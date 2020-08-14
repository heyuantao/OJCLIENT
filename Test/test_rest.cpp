//
// Created by he_yu on 2020-02-06.
//

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <boost/algorithm/string/replace.hpp>

#include "../Rest/Client.h"
#include "../Rest/Rest.h"
#include "../Common/Exceptions.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams
using namespace std;

std::vector<std::string> parseResponseString(std::string content) {
    std::vector<std::string> string_vector;
    std::istringstream stream(content);
    for (std::string item; stream >> item; ){
        string_vector.push_back(item);
    }
    return string_vector;
}

void test_acm(){
    vector<string> job_list;
    try {
        string url = "http://acm01e.syslab.org/admin/problem_judge.php";
        HttpFormData form;
        form.addItem("getpending", "1");
        form.addItem("oj_lang_set", "0,1,2,3");
        form.addItem("max_running", "10");

        string result = WgetRest::post(url, form).data;
        std::cout << "This is response !" << endl;
        std::cout << result;
    }catch(ClientNetworkException &e){
        std::cout<<"Network error happend"<<endl;
    }catch(...){
        std::cout<<"System error !"<<endl;
    }
}

void test_rest(){
    vector<string> job_list;
    try {
        string url = "http://172.16.16.180:8080/api/v1/onlinejudgeclient/test/";


        HttpFormData form; //form.addItem("getpending", "1");
        form.addItem("oj_lang_set", "0,1,2,3");
        form.addItem("max_running", "10");

        //string result = WgetRest::post(url, form).data;
        CasablancaRest::api_token="abc123";
        string result = CasablancaRest::post(url, form).data;
        //WgetRest::post(url, form).status
        std::cout << "This is response !" << endl;
        std::cout << result;

        std::cout << "This is response in line format!" << endl;
        vector<string> lines=parseResponseString(result);
        std::cout << lines.size() << endl;
        for(std::vector<std::string>::iterator item=lines.begin();item!=lines.end();item++){
            std::cout<<*item<<"###";
        }
        std::cout<<endl;

    }catch(ClientNetworkException &e){
        std::cout<<"Network error happend"<<endl;
    }catch(...){
        std::cout<<"System error !"<<endl;
    }
}

int main(){
    //test_acm();
    test_rest();
}