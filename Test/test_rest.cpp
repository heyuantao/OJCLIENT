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

void test_acm(){
    vector<string> job_list;
    try {
        string url = "http://acm01e.syslab.org/admin/problem_judge.php";
        HttpFormData form;
        form.addItem("getpending", "1");
        form.addItem("oj_lang_set", "0,1,2,3");
        form.addItem("max_running", "10");

        string result = WgetRest::post(url, form).data;
        cout << "This is response !" << endl;
        cout << result;
    }catch(ClientNetworkException &e){
        cout<<"Network error happend"<<endl;
    }catch(...){
        cout<<"System error !"<<endl;
    }
}

void test_rest(){
    vector<string> job_list;
    try {
        string url = "http://127.0.0.1:5001/api/v1/judgeclient/test/";
        HttpFormData form;
        form.addItem("getpending", "1");
        form.addItem("oj_lang_set", "0,1,2,3");
        form.addItem("max_running", "10");

        //string result = WgetRest::post(url, form).data;
        string result = CasablancaRest::post(url, form).data;
        //WgetRest::post(url, form).status
        cout << "This is response !" << endl;
        cout << result;
    }catch(ClientNetworkException &e){
        cout<<"Network error happend"<<endl;
    }catch(...){
        cout<<"System error !"<<endl;
    }
}

int main(){
    //test_acm();
    test_rest();
}