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

#include "Client.h"
#include "Rest.h"

using namespace std;

class HustojClient : public Client{
private:
    static const std::string check_login_api;            // = "/admin/problem_judge.php";
    static const std::string problem_judge_api;          // = "/admin/problem_judge.php";
//    static const std::vector<std::string> language_extension ;

private:
    std::string site_base_url = "";

//public:
//    static std::string getLanguageExtensionById(int lang);


public:
    /**
     * 构造函数
     * @param site_base_url
     */
    HustojClient(const std::string &site_base_url="http://127.0.0.1");

    /**
     * 析构函数
     */
    virtual ~HustojClient();

    /**
     * 仅仅在API请求时设置token，不会抛出异常
     * @param token
     */
    virtual void login(const std::string &token);

    /**
     * 检查是否已经登录，会抛出异常
     * @return
     */
    virtual bool checkLogin();

    /**
     * 会抛出异常
     * @param solution
     * @param result
     * @param time
     * @param memory
     * @param pass_rate
     */
    virtual void updateSolution(const std::string &solution, int result, int time, int memory, double pass_rate);

    /**
     * 会抛出异常
     * @param solution
     * @param work_dir
     * @param language
     */
    virtual void getSolution(const std::string &solution, std::string work_dir, int language);

    /**
     * 会抛出异常
     * @param solution
     * @param problem
     * @param username
     * @param lang
     */
    virtual void getSolutionInformation(const std::string &solution, std::string &problem,std::string &username,int &lang);

    /**
     * 会抛出异常
     * @param solution
     */
    virtual void addCompileErrorInformation(const std::string &solution);

    /**
     * 会抛出异常
     * @param solution
     */
    virtual void addRuningErrorInformation(const std::string &solution);


/*
    void addDifferentInformation(int solution);
    void addCustomOutputInformation(int solution);
    void getCustomInput(int solution,std::string work_dir);
*/

    /**
     * 会抛出异常
     * @param username
     */
    virtual void updateUserInformation(const std::string username);

    /**
     * 会抛出异常
     * @param problem
     * @param time_limit
     * @param mem_limit
     * @param isspj
     */
    virtual void getProblemInformation(const std::string &problem, int &time_limit, int &mem_limit, int &isspj);

    /**
     * 会抛出异常
     * @param problem
     */
    virtual void updateProblemInformation(const std::string &problem);

    /**
     * 会抛出异常
     * @return
     */
    virtual std::vector<std::string> getJobs();

    /**
     * 会抛出异常
     * @param problem
     * @param data_dir
     */
    virtual void getTestFile(const std::string &problem,std::string data_dir);


private:
    /**
     * 会抛出异常
     * @param problem
     * @param data_list
     */
    void getTestDataList(const std::string &problem, std::vector<std::string> &data_list);

    /**
     * 会抛出异常
     * @param problem
     * @param filename
     * @param date
     */
    void getTestDataDate(const std::string &problem, std::string filename, std::string &date);

    /**
     * 会抛出异常
     * @param problem
     * @param filename
     * @param filecontent
     */
    void getTestDataData(const std::string &problem, std::string filename, std::string & filecontent);

    /**
     * 不会抛出异常
     * @param dir_path_string
     */
    void createDirectoryIfNotExists(const std::string &dir_path_string);


private:
    /**
     * 获取api的地址
     * @param api
     * @return
     */
    std::string getHttpApiUrl(const std::string &api);

    /**
     * 解析返回的内容
     * @param content
     * @return
     */
    std::vector<string> parseResponseString(std::string content);

    /**
     * 读取命令行的输出结果
     * @param command
     * @return
     */
    Response readCommandOutput(std::string command);

    /**
     * 进行数据请求
     * @param url
     * @param form_data
     * @return
     */
    Response post(std::string url,HttpFormData form_data);

    /**
     * url地址编码
     * @param origin
     * @return
     */
    std::string url_encoder(const std::string &origin);

};


#endif //OJCLIENT_REST_HUSTOJCLIENT_H
