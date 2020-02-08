//
// Created by he_yu on 2020-01-21.
//

#ifndef OJCLIENT_REST_HUSTOJCLIENT_H
#define OJCLIENT_REST_HUSTOJCLIENT_H

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

class HustojClient{
private:
    static const std::string check_login_api;            // = "/admin/problem_judge.php";
    static const std::string problem_judge_api;          // = "/admin/problem_judge.php";
//    static const std::vector<std::string> language_extension ;

private:
    std::string site_base_url = "";

//public:
//    static std::string getLanguageExtensionById(int lang);

public:
    HustojClient(const std::string &site_base_url="http://127.0.0.1");
    virtual ~HustojClient();

    virtual void login(const string &token);                                                            //no exception, just set the api token
    virtual bool checkLogin();                                                                          //exception, send the api to check if is login
    virtual void updateSolution(int solution,int result,int time,int memory,double pass_rate);          //exception
    virtual void getSolution(int solution,std::string work_dir,int language);                           //exception
    virtual void getSolutionInformation(int solution,int &problem,std::string &username,int &lang);     //exception
    virtual void addCompileErrorInformation(int solution);                                              //exception
    virtual void addRuningErrorInformation(int solution);                                               //exception
    //void addDifferentInformation(int solution);
    //void addCustomOutputInformation(int solution);
    //void getCustomInput(int solution,std::string work_dir);
    virtual void updateUserInformation(const std::string username);                                     //exception
    virtual void getProblemInformation(int problem, int &time_limit, int &mem_limit, int &isspj);       //exception
    virtual void updateProblemInformation(int problem);                                                 //exception
    virtual std::vector<std::string> getJobs();                                                         //exception
    virtual void getTestFile(int problem,std::string data_dir);                                         //exception

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


#endif //OJCLIENT_REST_HUSTOJCLIENT_H
