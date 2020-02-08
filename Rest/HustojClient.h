//
// Created by he_yu on 2020-01-21.
//

#ifndef HUSTOJCLIENT_H
#define HUSTOJCLIENT_H

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>

#include "./Rest.h"

using namespace std;

class Client{
private:
    //static const std::string login_api;
    static const std::string check_login_api;            // = "/admin/problem_judge.php";
    static const std::string problem_judge_api;          // = "/admin/problem_judge.php";
    static const std::vector<std::string> language_extension ;

private:
    std::string site_base_url = "";

public:
    static std::string getLanguageExtensionById(int lang);

public:
    Client(const std::string &site_base_url="http://127.0.0.1");

    void login(const string &token);                                                            //no exception, just set the api token
    bool checkLogin();                                                                          //exception, send the api to check if is login
    void updateSolution(int solution,int result,int time,int memory,double pass_rate);          //exception
    void getSolution(int solution,std::string work_dir,int language);                           //exception
    void getSolutionInformation(int solution,int &problem,std::string &username,int &lang);     //exception
    void addCompileErrorInformation(int solution);                                              //exception
    void addRuningErrorInformation(int solution);                                               //exception
    //void addDifferentInformation(int solution);
    //void addCustomOutputInformation(int solution);
    //void getCustomInput(int solution,std::string work_dir);
    void updateUserInformation(const std::string username);                                     //exception
    void getProblemInformation(int problem, int &time_limit, int &mem_limit, int &isspj);       //exception
    void updateProblemInformation(int problem);                                                 //exception
    std::vector<std::string> getJobs();                                                         //exception
    void getTestFile(int problem,std::string data_dir);                                         //exception

private:
    void getTestDataList(int problem, std::vector<std::string> &data_list);                           //exception
    void getTestDataDate(int problem, std::string filename, std::string &date);                       //exception
    void getTestDataData(int problem, std::string filename, std::string & filecontent);               //exception
    void createDirectoryIfNotExists(const std::string &dir_path_string);

private:
    std::string getHttpApiUrl(const std::string &api);
    std::vector<string> parseResponseString(std::string content);
    Response readCommandOutput(std::string command);
    Response post(std::string url,HttpFormData form_data);
    std::string url_encoder(const std::string &origin);

};


#endif //OJCLIENT_CLIENT_H
