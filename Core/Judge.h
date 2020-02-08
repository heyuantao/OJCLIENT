//
// Created by he_yu on 2020-01-27.
//

#ifndef OJCLIENT_JUDGE_H
#define OJCLIENT_JUDGE_H

#include <string>

#include <unistd.h>
#include <wait.h>
#include <sys/ptrace.h>

#include "../Rest/Client.h"
#include "../Common/Utils.h"

#define STD_MB 1048576
#define STD_F_LIM (STD_MB<<5)
#define STD_M_LIM (STD_MB<<7)
#define REG_SYSCALL orig_rax

using namespace std;

class JudgeResult{
public:
    const static int WT0=0;
    const static int WT1=1;
    const static int CI=2;
    const static int RI=3;
    const static int AC=4;
    const static int PE=5;
    const static int WA=6;
    const static int TL=7;
    const static int ML=8;
    const static int OL=9;
    const static int RE=10;
    const static int CE=11;
    const static int CO=12;
    const static int TR= 13;
public:
    //This function is use by logger for debug
    static std::string getStatusDescribeById(int status_id){
        std::string status="";
        switch (status_id){
            case 0: status="WT0"; break;
            case 1: status="WT1"; break;
            case 2: status="CI";  break;
            case 3: status="RI";  break;
            case 4: status="AC";  break;
            case 5: status="PE";  break;
            case 6: status="WA";  break;
            case 7: status="TL";  break;
            case 8: status="ML";  break;
            case 9: status="OL";  break;
            case 10:status="RE";  break;
            case 11:status="CE";  break;
            case 12:status="CO";  break;
            case 13:status="TR";  break;
        }
        return status;
    }
};

class Judge{
private:
    std::string solution;
    std::string task;
    std::string judge_home_path;
    std::string judge_work_path;
    std::string judge_data_path;
    //use by judge parts
    std::vector<int> call_counter;        //call_counter init with 256 zero
private:
    ClientSettings *settings=NULL;
    Client *client=NULL;
public:
    Judge(std::string judge_home_path="/home/judge", std::string solution="1000", std::string task="0");
    ~Judge();
    void run();
    bool compareFile(const std::string &source_file_path, const std::string &dist_file_path);
    string processString(const std::string &content);
    void cleanWorkDirectory(const std::string &dir);

private:
    void executeSystemCommand(const std::string &cmd);
    void handleLangageResource(int lang, int &time_limit, int &memory_limit);
    bool compile(int lang,const std::string &workdir);
    void executeExecvp(const std::string &cmd);
    long getFileSize(const std::string &file_path);
    void copyRuntime(int lang,const std::string &workdir);
    void copyShellRuntime(const std::string &workdir);
    void copyBashRuntime(const std::string  &workdir);
    void copyPythonRuntime(const std::string &workdir);
    void copyRubyRuntime(const std::string &workdir);        //not implement
    void copyPhpRuntime(const std::string &workdir);         //not implement
    void copyPerlRuntime(const std::string &workdir);        //not implement
    void copyMonoRuntime(const std::string &workdir);        //not implement
    void copyObjcRuntime(const std::string &workdir);        //not implement
    void copyFreebasicRuntime(const std::string &workdir);   //not implement
    void copyGuileRuntime(const std::string &workdir);       //not implement
    void copyLuaRuntime(const std::string &workdir);         //not implement
    void copyJsRuntime(const std::string &workdir);          //not implement
    void initSafeSysCall(int lang);
    std::vector<std::string> getTestFileListFromLocal(int problem);
    void prepareTestFile(int task, int problem, const string &test_file_name);
    void runSolution(int task, int lang, int time_limit, int memory_limit, int used_time);
    void watchSolution(pid_t process_id, int task, int lang,  int &ac_flag, int &memory_peak, int memory_limit, int &used_time, int time_limit, const std::string &test_file_name);
    int getProcessStatus(pid_t process_id, std::string key_name);
    int getPageFaultMemory(struct rusage &resource_usage);
    void judgeSolution(int &ac_flag, int problem, int lang, int &memory_peak, int memory_limit, int &used_time, int time_limit, const std::string &test_file_name);
    void fixJavaMistakeJudge(int &ac_flag, int &memory_peak, int memory_limit);
    void fixPythonMistakeJudge(int &ac_flag, int &memory_peak, int memory_limit);
};

#endif //OJCLIENT_JUDGE_H
