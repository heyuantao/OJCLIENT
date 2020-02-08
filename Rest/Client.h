//
// Created by he_yu on 2020-01-21.
//

#ifndef OJCLIENT_REST_CLIENT_H
#define OJCLIENT_REST_CLIENT_H

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>

#include "./Rest.h"

class Client{
public:
    static const std::vector<std::string> language_extension ;
public:
    static std::string getLanguageExtensionById(int lang);
public:
    Client(const std::string &site_base_url="http://127.0.0.1");
    virtual ~Client();
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
};


#endif //REST_CLIENT_H
