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

/**
 * OJ客户端的接口类，与API服务器进行互交
 */
class Client{

public:
    /**
     * 存放语言的支持集合，以扩展名来存放
     */
    static const std::vector<std::string> language_extension ;


public:
    /**
     * 每个语言有一个id编号，该函数通过编号来获取语言的扩展名
     * @param lang
     * @return
     */
    static std::string getLanguageExtensionById(int lang) ;

    /**
     * 工厂构建函数
     * @param client_name
     * @param base_url
     * @return
     */
    static Client* createClient(const string &client_name,const string &base_url);


public:

    /**
     * 初始化该对象，
     * @param site_base_url，在构造函数中设置了默认的地址，实际使用中要更换掉
     */
    Client(const std::string &site_base_url="http://127.0.0.1");

    /**
     * 清理该对象
     */
    virtual ~Client();

    /**
     * no exception, just set the api token
     * @param token
     */
    virtual void login(const string &token);

    /**
     * exception, send the api to check if is login
     * @return
     */
    virtual bool checkLogin();

    /**
     * 更新判题结果，会抛出异常
     * @param solution
     * @param result
     * @param time
     * @param memory
     * @param pass_rate
     */
    virtual void updateSolution(const std::string &solution,int result,int time,int memory,double pass_rate);

    /**
     * 获取题目的信息，会抛出异常
     * @param solution
     * @param work_dir
     * @param language
     */
    virtual void getSolution(const std::string &solution,std::string work_dir,int language);

    /**
     * 获取提交的信息，会抛出异常
     * @param solution
     * @param problem
     * @param username
     * @param lang
     */
    virtual void getSolutionInformation(const std::string &solution,const std::string &problem,std::string &username,int &lang);

    /**
     * 添加编译阶段的信息，会抛出异常
     * @param solution
     */
    virtual void addCompileErrorInformation(const std::string &solution);

    /**
     * 添加运行时的信息，会抛出异常
     * @param solution
     */
    virtual void addRuningErrorInformation(const std::string & solution);

    //void addDifferentInformation(int solution);
    //void addCustomOutputInformation(int solution);
    //void getCustomInput(int solution,std::string work_dir);

    /**
     * 更新用户的信息，即某个用户的做题情况，实际不会使用，会抛出异常
     * @param username
     */
    virtual void updateUserInformation(const std::string username);

    /**
     * 获得试题的信息，会抛出异常
     * @param problem
     * @param time_limit
     * @param mem_limit
     * @param isspj
     */
    virtual void getProblemInformation(const std::string &problem, int &time_limit, int &mem_limit, int &isspj);

    /**
     * 更新该题目的统计数据，实际不会遇到，会抛出异常
     * @param problem
     */
    virtual void updateProblemInformation(const std::string &problem);

    /**
     * 获得要执行任务的信息，会抛出异常
     * @return
     */
    virtual std::vector<std::string> getJobs();

    /**
     * 获取测试数据，会抛出异常
     * @param problem
     * @param data_dir
     */
    virtual void getTestFile(const std::string &problem,std::string data_dir);

};


#endif //REST_CLIENT_H
