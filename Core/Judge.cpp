//
// Created by he_yu on 2020-01-27.
//
#include <sstream>
#include <string>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/user.h>

#include "Judge.h"
#include "SafeSysCall.h"
#include "../Common/Utils.h"
#include "../Common/Exceptions.h"
#include "../Common/Logger.h"
#include "../Rest/Client.h"


using namespace std;

Judge::Judge(const std::string &judge_home_path, const std::string &solution, const std::string &task, bool debug) {
    this->solution = solution;
    this->task = task;
    this->debug = debug;                                    //if true ,the program will display debug message and not clear the task working directory

    boost::filesystem::path judge_home(judge_home_path);
    this->judge_home_path = judge_home.string();

    boost::filesystem::path judge_work(judge_home_path);
    judge_work /= string("run")+this->task;
    this->judge_work_path = judge_work.string();

    boost::filesystem::path judge_data(judge_home_path);
    judge_data /= "data";
    this->judge_data_path = judge_data.string();

    this->call_counter = vector<int>(512,0);

    this->settings = NULL;
    this->client = NULL;
}

void Judge::init(){
    try{
        this->settings = new ClientSettings("/home/judge/etc/judge.conf");
        this->settings->init();

        //this->client = new Client(this->settings->BASEURL);
        this->client = Client::createClient("HustojClient",this->settings->BASEURL);
    }catch (ClientException &e){
        std::stringstream ss;
        e.printException();
        ss << boost::format("Exception happen in Judge::init()") << "";
        throw ClientMessageException(ss.str());
    }catch (...){
        std::stringstream ss;
        ss << boost::format("Unknow exception happen in Judge::init()") << "";
        throw ClientMessageException(ss.str());
    }
}

Judge::~Judge() {
    delete this->client;
    this->client=NULL;

    delete this->settings;
    this->settings=NULL;
}

void Judge::run() {
    int solution,problem,lang;                      //solution id,problem id,lang
    std::string username;                                //user_id
    int time_limit,memory_limit,is_special_judge;   //problem other settings


    ClientLogger::DEBUG("Login and checkLogin in Judge::run()");
    this->client->login(this->settings->API_KEY);
    if(!this->client->checkLogin()){
        ClientLogger::ERROR("Can not login in Daemon::run()");
        return;
    };

    ClientLogger::DEBUG("Clear workdir for solution id "+this->solution+" in Judge::run()");
    this->cleanWorkDirectory(this->judge_work_path);        //clear /home/judge/run0  0 is sampe as task
    chdir(this->judge_work_path.c_str());

    ClientLogger::DEBUG("Get solution id "+this->solution+" information in Judge::run()");
    solution= std::stoi(this->solution);
    this->client->getSolutionInformation(solution,problem,username,lang);

    ClientLogger::DEBUG("Check if solution id "+this->solution+" is in language set Judge::run()");
    if(this->settings->isInLanguageSet(lang)== false){
        ClientLogger::ERROR("Solution id "+this->solution+" not in support language set \""+this->settings->LANG_SET+"\" Judge::run()");
        this->client->updateSolution(solution, JudgeResult::WA, 0, 0, 0.0);
        return;
    }

    ClientLogger::DEBUG("Get problem id "+to_string(problem)+" information in Judge::run()");
    this->client->getProblemInformation(problem,time_limit,memory_limit,is_special_judge);

    //get solution
    ClientLogger::DEBUG("Get solution id "+this->solution+" source code into dir in Judge:run()");
    this->client->getSolution(solution,this->judge_work_path,lang);

    ClientLogger::DEBUG("Prepare resource and java policy file for solution id "+this->solution+" Judge::run()");
    this->handleLangageResource(lang,time_limit,memory_limit);

    //compile the source code
    ClientLogger::DEBUG("Compile solution id "+this->solution+" source file in Judge::run()");
    bool compile_status = this->compile(lang, this->judge_work_path);
    if(compile_status== false){
        this->client->addCompileErrorInformation(solution);
        this->client->updateSolution(solution, JudgeResult::CE, 0, 0, 0.0);
        ClientLogger::DEBUG("Compile solution id "+this->solution+" source code error in Judge::run()");
        return;
    }else{
        this->client->updateSolution(solution, JudgeResult::RI, 0, 0, 0.0);
        ClientLogger::DEBUG("Compile solution id "+this->solution+"source code finished in Judge::run()");
        //umount(this->judge_work_path.c_str());  // This directory is not mount why do this ?
    }

    //get the test file for problem
    ClientLogger::DEBUG("Get problem id "+to_string(problem)+" test file in Judge::run()");
    this->client->getTestFile(problem,this->judge_data_path);

    //run solution and judge it
    ClientLogger::DEBUG("Run solution id "+this->solution+" in Judge::run()");
    int used_time = 0;
    int memory_peak = 0;
    //int num_of_test = 0;                //number of test file
    double pass_rate = 0.0;             //calc the rate of test,this value is add on every success test,and divide by number_of_test finally
    int ac_flag = JudgeResult::AC;      //init pe_flag = OJ_AC;
    int final_ac_flag = ac_flag;        //this is oj status submit to server

    std::vector<std::string> test_in_file_list = this->getTestFileListFromLocal(problem);
    for(std::vector<std::string>::iterator itr = test_in_file_list.begin(); itr!=test_in_file_list.end(); itr++){
        ClientLogger::DEBUG("Prepare input test \""+*itr+"\" file in Judge::run()");
        this->prepareTestFile(stoi(this->task), problem, *itr);
        this->initSafeSysCall(lang);

        pid_t process_pid = fork();
        if(process_pid==0){
            ClientLogger::DEBUG("Run solution id "+this->solution+" on test file in Judge::run()");
            this->runSolution(stoi(this->task), lang, time_limit, memory_limit, used_time);
        }else{
            //num_of_test = num_of_test +1;
            ClientLogger::DEBUG("Watch solution id "+this->solution+" in Judge::run()");
            this->watchSolution(process_pid,stoi(this->task),lang,ac_flag,memory_peak,memory_limit,used_time,time_limit,*itr);
            ClientLogger::DEBUG("Judge solution id "+this->solution+" in Judge::run()");
            this->judgeSolution(ac_flag,problem,lang,memory_peak,memory_limit,used_time,time_limit,*itr);
            //judge solution
            ClientLogger::DEBUG("Judge result on this test file is "+JudgeResult::getStatusDescribeById(ac_flag)+" in Judge:run()");
        }
        if(ac_flag==JudgeResult::AC){
            pass_rate = pass_rate+1;
        }
        if(final_ac_flag<ac_flag){
            final_ac_flag = ac_flag;
        }
        ac_flag = JudgeResult::AC;
    }//end for loop on every test file

    if((ac_flag==JudgeResult::RE)||(final_ac_flag==JudgeResult::RE)){
        ClientLogger::DEBUG("Judge result to solution id "+this->solution+" is RE in Judge::run()");
        this->client->addRuningErrorInformation(solution);
    }
    if(ac_flag==JudgeResult::TL){
        used_time = time_limit*1000;
    }
    if(test_in_file_list.size()>0){                         //calc pass passing rate on every test file
        pass_rate = pass_rate/test_in_file_list.size();     //test_in_file_list.size() is the number of test file
    }
    this->client->updateSolution(solution,final_ac_flag,used_time,memory_peak>>10,pass_rate);
    ClientLogger::DEBUG("The final judge result to solution id "+this->solution+" is "+JudgeResult::getStatusDescribeById(final_ac_flag)+" and the pass rate is "+to_string(pass_rate)+" in Judge:run()");
    if((ac_flag==JudgeResult::WA)&&(final_ac_flag==JudgeResult::WA)){
        // add diff info latter ,work to be done here
    }
    //clear the work dir
    //this->cleanWorkDirectory(this->judge_work_path);
    ClientLogger::DEBUG("Clear work for solution id "+this->solution+" in Judge:run()");
}

void Judge::cleanWorkDirectory(const std::string &dir) {
    try{
        std::stringstream ss;
        umount(dir.c_str());

        //create the workdir and set permission
        boost::filesystem::path work_dir(dir);
        if(!boost::filesystem::exists(work_dir)){
            boost::filesystem::create_directory(work_dir);      //create /home/judge/runx
            uid_t work_dir_uid = this->settings->process_uid;
            gid_t work_dir_gid = this->settings->process_gid;
            chown(work_dir.c_str(),work_dir_uid,work_dir_gid);    // chown judge:judge on /home/judge/runx
        }

        ss.str("");
        ss << boost::format("/bin/rm -rf %s/*") % dir<< '\n';
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("mkdir %s/log") % dir <<'\n';
        this->executeSystemCommand(ss.str());

    }catch (ClientRunCommandException &e){
        std::stringstream ss;
        ss << boost::format("Exception Happend in Judge::cleanWorkDirectory !") <<"";
        e.printException();
        throw ClientAppException(ss.str());
    }
}

void Judge::executeSystemCommand(const std::string &cmd) {
    int return_status = system(cmd.c_str());
    if(return_status!=0){
        std::stringstream ss;
        ss << boost::format("Can not run: \"%s\" in Judge::executeSystemCommand()") % cmd << "";
        throw ClientRunCommandException(ss.str());
    }
}

void Judge::executeSystemCommandWithoutException(const std::string &cmd) {
    int return_status = system(cmd.c_str());
}

//remove '\r' char and reduce multi space and table to one space
std::string Judge::processString(const std::string &content) {
    std::istringstream iss(content);
    std::string line;
    std::stringstream line_ss;
    std::stringstream content_ss;

    char char_in_string;

    while(std::getline(iss,line)){
        line = boost::algorithm::erase_all_copy(line, "\r");    //remove '\r' character in every line

        if(line.length()==0){
            continue;
        }

        line_ss.str("");                                    // clear the stringstream content
        bool latest_char_is_space = false;
        for(int i=0;i<line.length();i++){
            char_in_string = line[i];
            if( (char_in_string==' ')&&(latest_char_is_space== false)){
                line_ss << ' ';
                latest_char_is_space = true;
                continue;
            }
            if( (char_in_string=='\t')&&(latest_char_is_space== false)){
                line_ss << ' ';
                latest_char_is_space = true;
                continue;
            }
            if( (char_in_string==' ')&&(latest_char_is_space== true) ){
                latest_char_is_space = true;
                continue;
            }
            if( (char_in_string=='\t')&&(latest_char_is_space== true) ){
                latest_char_is_space = true;
                continue;
            }
            if( char_in_string!=' ' ){
                line_ss << char_in_string;
                latest_char_is_space = false;
                continue;
            }
            if( (char_in_string == '\n')||(char_in_string == '\r')){
                latest_char_is_space = false;
                continue;
            }
        }

        content_ss << line_ss.str() << endl;

    }
    return content_ss.str();
}

bool Judge::compareFile(const std::string &source_file_path, const std::string &dist_file_path) {
    std::ifstream source_stream;
    std::ifstream dist_stream;
    std::stringstream ss_source,ss_dist;
    try{
        boost::filesystem::path source_path(source_file_path);
        boost::filesystem::path dist_path(dist_file_path);
        if( ( !boost::filesystem::exists(source_path) )||( !boost::filesystem::exists(dist_path) ) ){
            ClientLogger::DEBUG("File not found in Judge::compareFile");
            throw ClientFileException("File not found in Judge::compareFile");
        }

        ss_source.str("");
        source_stream.open(source_file_path);
        ss_source << source_stream.rdbuf();
        std::string source_file_content = ss_source.str();
        std::string processed_source_file_content = this->processString(source_file_content);
        source_stream.close();

        ss_dist.str("");
        dist_stream.open(dist_file_path);
        ss_dist << dist_stream.rdbuf();
        std::string dist_file_content = ss_dist.str();
        std::string processed_dist_file_content = this->processString(dist_file_content);
        dist_stream.close();

        /*
        std::cout<<"file 1:"<<source_file_path<<endl;
        std::cout<<processed_source_file_content;
        std::cout<<"file 2:"<<dist_file_path<<endl;
        std::cout<<processed_dist_file_content;
        */

        //two file is different
        if(processed_source_file_content.length()!=processed_dist_file_content.length()){
            return false;
        }
        for(int i=0;i<processed_dist_file_content.length();i++){
            char source_file_char = processed_source_file_content[i];
            char dist_file_char = processed_dist_file_content[i];
            if(source_file_char!=dist_file_char){
                return false;
            }
        }
        //two file have same content
        return true;
    }catch (...){
        source_stream.close();
        dist_stream.close();
        ClientLogger::ERROR("Exception in compare file !");
        return false;
    }
}

void Judge::handleLangageResource(int lang, int &time_limit, int &memory_limit) {
    time_limit = time_limit + std::stoi(this->settings->JAVA_TIME_BONUS);
    memory_limit = memory_limit + std::stoi(this->settings->JAVA_MEMORY_BONUS);

    if(time_limit > 300|| time_limit< 1){
        time_limit = 300;
    }
    if(memory_limit > 1024 || memory_limit <1 ){
        memory_limit = 1024;
    }

    std::stringstream ss;
    try{
        if(this->client->getLanguageExtensionById(lang)=="java"){
            ss.str("");
            ss << boost::format("/bin/cp %s/etc/java0.policy %s/java.policy") % this->judge_home_path % this->judge_work_path << "";
            this->executeSystemCommand(ss.str());

            ss.str("");
            ss << boost::format("chmod 755 %s/java.policy") % this->judge_work_path << "";
            this->executeSystemCommand(ss.str());

            ss.str("");
            ss << boost::format("chown judge %s/java.policy") % this->judge_work_path << "";
            this->executeSystemCommand(ss.str());
        }
    }catch (ClientException &e){
        e.printException();
        ss.str("");
        ss << boost::format("Execute command error in Judge::handleLangageResource()") << "";
        throw ClientAppException(ss.str());
    }
}

bool Judge::compile(int lang, const std::string &workdir) {
    std::stringstream ss;
    
    const std::string compile_c = "gcc Main.c -o Main -fno-asm -Wall -lm --static -std=c99 -DONLINE_JUDGE";
    const std::string compile_cpp = "g++ -fno-asm -Wall -lm --static -std=c++11 -DONLINE_JUDGE -o Main Main.cc";
    const std::string compile_pascal = "fpc Main.pas -Cs32000000 -Sh -O2 -Co -Ct -Ci";
    const std::string compile_ruby = "ruby -c Main.rb";
    const std::string compile_bash = "chmod +rx Main.sh";
    const std::string compile_php =  "php -l Main.php";
    const std::string compile_perl = "perl -c Main.pl";
    const std::string compile_csharp = "gmcs -warn:0 Main.cs";
    const std::string compile_objectc = "gcc -o Main Main.m -fconstant-string-class=NSConstantString -I /usr/include/GNUstep/ -L /usr/lib/GNUstep/Libraries/ -lobjc -lgnustep-base";
    const std::string compile_basic = "fbc -lang qb Main.bas";
    const std::string compile_clang = "clang Main.c -o Main -fno-asm -Wall -lm --static -std=c99 -DONLINE_JUDGE";
    const std::string compile_clang_cpp = "clang++ Main.cc -o Main -fno-asm -Wall -lm --static -std=c++0x -DONLINE_JUDGE";
    const std::string compile_lua = "luac -o Main Main.lua";
    const std::string compile_go = "go build -o Main Main.go";

    ss.str("");
    ss << boost::format("javac -J%s -J%s -encoding UTF-8 Main.java") % this->settings->JAVA_XMS % this->settings->JAVA_XMX << "";
    const std::string compile_java = ss.str();

    // begin the compile in child process
    pid_t process_pid = fork();
    if(process_pid==0){
        //recource struct
        struct rlimit LIM;

        //cpu resource limit
        LIM.rlim_max = 6;
        LIM.rlim_cur = 6;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(6);

        //file resource limit
        LIM.rlim_max = 10 * STD_MB;
        LIM.rlim_cur = 10 * STD_MB;
        setrlimit(RLIMIT_FSIZE, &LIM);

        //memory resource limit
        if(lang==3||lang==17){
            LIM.rlim_max = STD_MB <<11;
            LIM.rlim_cur = STD_MB <<11;
        }else{
            LIM.rlim_max = STD_MB *256 ;
            LIM.rlim_cur = STD_MB *256 ;
        }
        setrlimit(RLIMIT_AS, &LIM);

        if (lang != 2 && lang != 11) {
            freopen("ce.txt", "w", stderr);
        } else {
            freopen("ce.txt", "w", stdout);
        }

        // compile in chroot mode
        if(lang != 3 && lang != 9 && lang != 6 && lang != 11){
            this->executeSystemCommand("mkdir -p bin usr lib lib64 etc/alternatives proc tmp dev");
            this->executeSystemCommand("chown judge *");
            this->executeSystemCommand("mount -o bind /bin bin");
            this->executeSystemCommand("mount -o bind /usr usr");
            this->executeSystemCommand("mount -o bind /lib lib");
            this->executeSystemCommand("mount -o bind /lib64 lib64");
            this->executeSystemCommand("mount -o bind /etc/alternatives etc/alternatives");
            this->executeSystemCommand("mount -o bind /proc proc");
            if(lang>2 && lang!=10 && lang!=13 && lang!=14)
                this->executeSystemCommand("mount -o bind /dev dev");
            chroot(this->judge_work_path.c_str());
            //chdir("/");        //hyt add this and work with chroot
        }

        uid_t work_dir_uid = this->settings->process_uid;
        gid_t work_dir_gid = this->settings->process_gid;
        while(setgid(work_dir_gid)!=0) sleep(1);
        while(setuid(work_dir_uid)!=0) sleep(1);
        while(setresuid(work_dir_uid, work_dir_uid, work_dir_uid)!=0) sleep(1);


        switch (lang) {
            case 0:
                this->executeExecvp(compile_c);
                break;
            case 1:
                this->executeExecvp(compile_cpp);
                break;
            case 2:
                this->executeExecvp(compile_pascal);
                break;
            case 3:
                this->executeExecvp(compile_java);
                break;
            case 4:
                this->executeExecvp(compile_ruby);
                break;
            case 5:
                this->executeExecvp(compile_bash);
                break;
            //case 6:
            //	execvp(python);
            //	break;
            case 7:
                this->executeExecvp(compile_php);
                break;
            case 8:
                this->executeExecvp(compile_perl);
                break;
            case 9:
                this->executeExecvp(compile_csharp);
                break;
            case 10:
                this->executeExecvp(compile_objectc);
                break;
            case 11:
                this->executeExecvp(compile_basic);
                break;
            case 13:
                this->executeExecvp(compile_clang);
                break;
            case 14:
                this->executeExecvp(compile_clang_cpp);
                break;
            case 15:
                this->executeExecvp(compile_lua);
                break;
            //case 16:
            //	execvp(CP_JS[0], (char * const *) CP_JS);
            //	break;
            case 17:
                this->executeExecvp(compile_go);
                break;
            default:
                ss.str("");
                ss << boost::format("Not support language id %s") % lang << "";
                ClientLogger::ERROR(ss.str());
        }
        ClientLogger::DEBUG("Compile Finished");
        exit(0);
    }else{
        int status;
        waitpid(process_pid, &status, 0);
        if (lang > 3 && lang < 7){
            status = this->getFileSize("ce.txt");           //getFileSize return long but this line not convert
        }
        //this->executeSystemCommand("/bin/umount -f bin usr lib lib64 etc/alternatives proc dev 2>&1 >/dev/null");


        this->executeSystemCommandWithoutException("/bin/umount -f bin usr lib lib64 etc/alternatives proc 2>&1 >/dev/null");
        if(lang>2 && lang!=10 && lang!=13 && lang!=14){
            this->executeSystemCommandWithoutException("/bin/umount -f dev 2>&1 >/dev/null");
        }

        //ss.str("");
        //ss << boost::format("/bin/umount -f %s/* 2>&1 >/dev/null") % this->judge_work_path << "";
        //this->executeSystemCommand(ss.str());
        umount(this->judge_work_path.c_str());

        if(status==0){          //if status is zero ,is means waitpid get error wait status or ce.txt has content
            return true;
        }else{
            return false;
        }
    }
    //this->executeExecvp(compile_c);
}

void Judge::executeExecvp(const std::string &command_string) {
    std::vector<std::string> cmd_string_vector ;
    boost::algorithm::split(cmd_string_vector, command_string, boost::algorithm::is_space());
    std::vector<char *> cmd_char_list_array;
    cmd_char_list_array.reserve(cmd_string_vector.size()+1);                                //The last element is NULL or zero ,reverse space for it

    for(std::vector<std::string>::iterator itr = cmd_string_vector.begin(); itr != cmd_string_vector.end(); itr++){
        cmd_char_list_array.push_back(const_cast<char *>(itr->c_str()));
    }
    cmd_char_list_array.push_back(NULL);                                                        //Push NULL or zero to the last element , NULL work for test

    if(cmd_char_list_array.empty()){
        std::stringstream ss;
        ss << boost::format("Command %s is invalid in Judge::executeExecvp") % command_string <<"";
        throw ClientRunCommandException(ss.str());
    }

    /*
    cout<<"Exec command !"<<endl;
    for(int i=0;i<cmd_string_vector.size()+1;i++){
        cout<<cmd_char_list_array[i]<<" ";
    }
     cout << endl;
     */

    execvp(cmd_char_list_array[0],cmd_char_list_array.data());  //need test in this line
}

long Judge::getFileSize(const std::string &file_path) {
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) == -1) {
        return 0;
    }
    return (long) file_stat.st_size;
}

void Judge::copyRuntime(int lang, const std::string &workdir) {
    std::stringstream ss;
    try{
        if (lang == 4){
            this->copyRubyRuntime(workdir);
        }
        if (lang == 5){
            this->copyBashRuntime(workdir);
        }
        if (lang == 6){
            this->copyPythonRuntime(workdir);
        }
        if (lang == 7){
            this->copyPhpRuntime(workdir);
        }
        if (lang == 8){
            this->copyPerlRuntime(workdir);
        }
        if (lang == 9){
            this->copyMonoRuntime(workdir);
        }
        if (lang == 10){
            this->copyObjcRuntime(workdir);
        }
        if (lang == 11){
            this->copyFreebasicRuntime(workdir);
        }
        if (lang == 12){
            this->copyGuileRuntime(workdir);
        }
        if (lang == 15){
            this->copyLuaRuntime(workdir);
        }
        if (lang == 16){
            this->copyJsRuntime(workdir);
        }
    }catch (ClientNotImplementException &e){
        ss.str("");
        ss << boost::format("Language is %d is not support !") % lang << "";
        e.printException();
        throw ClientAppException(ss.str());
    }catch (ClientException &e){
        ss.str("");
        ss << boost::format("Exception happend in  Judge::copyRuntime !") << "";
        e.printException();
        throw ClientAppException(ss.str());
    }
}

void Judge::copyShellRuntime(const std::string &workdir) {
    std::stringstream  ss;
    try{
        ss.str("");
        ss << boost::format("/bin/mkdir %s/lib") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/mkdir %s/lib64") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/mkdir %s/bin") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/cp /lib64/* %s/lib64/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/cp /bin/busybox %s/bin/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/sh") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/cp /bin/bash %s/bin/bash") % workdir << "";
        this->executeSystemCommand(ss.str());
    }catch (ClientRunCommandException &e){
        ss.str("");
        ss << boost::format("Execute command exception in Judge::copyShellRunTime") <<"";
        e.printException();
        throw ClientRunCommandException(ss.str());
    }

}

void Judge::copyBashRuntime(const std::string &workdir) {
    this->copyShellRuntime(workdir);

    std::stringstream  ss;
    try{
        ss.str("");
        ss << boost::format("/bin/cp `which bc`  %s/bin/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("busybox dos2unix Main.sh") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/grep") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/awk") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/cp /bin/sed %s/bin/sed") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/cut") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/sort") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/join") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/wc") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/tr") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/dc") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/dd") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/cat") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/tail") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/head") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/ln -s /bin/busybox %s/bin/xargs") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("chmod +rx %s/Main.sh") % workdir << "";
        this->executeSystemCommand(ss.str());

    }catch (ClientException &e){
        ss.str("");
        ss << boost::format("Execute command exception in Judge::copyBashRuntime") <<"";
        e.printException();
        throw ClientAppException(ss.str());
    }
}

void Judge::copyPythonRuntime(const std::string &workdir) {
    this->copyShellRuntime(workdir);

    std::stringstream  ss;
    try {
        ss.str("");
        ss << boost::format("mkdir -p %s/usr/include") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("mkdir -p %s/dev") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("mkdir -p %s/usr/lib") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("mkdir -p %s/usr/lib64") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("mkdir -p %s/usr/local/lib") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("cp /usr/bin/python* %s/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("cp -a /usr/lib/python* %s/usr/lib/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("cp -a /usr/lib64/python* %s/usr/lib64/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("cp -a /usr/local/lib/python* %s/usr/local/lib/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("cp -a /usr/include/python* %s/usr/include/") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("cp -a /usr/lib/libpython* %s/usr/lib/") % workdir << "";
        this->executeSystemCommand(ss.str());

        /*
        ss.str("");
        ss << boost::format("/bin/mkdir -p %s/home/judge") % workdir << "";             //import
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/chown judge %s") % workdir << "";                     //import
        this->executeSystemCommand(ss.str());
        */

        ss.str("");
        ss << boost::format("/bin/mkdir -p %s/etc") % workdir << "";
        this->executeSystemCommand(ss.str());

        ss.str("");
        ss << boost::format("/bin/grep judge /etc/passwd>%s/etc/passwd") % workdir << "";
        this->executeSystemCommand(ss.str());

        /*
        ss.str("");
        ss << boost::format("/bin/mount -o bind /dev %s/dev") % workdir << "";          //import
        this->executeSystemCommand(ss.str());
        */
    }catch(ClientRunCommandException &e){
        ss.str("");
        ss << boost::format("Execute command exception in Judge::copyPythonRuntime") <<"";
        e.printException();
        throw ClientRunCommandException(ss.str());
    }
}

void Judge::copyRubyRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Ruby is not support !");
}

void Judge::copyPhpRuntime(const std::string &workdir) {
    throw ClientNotImplementException("PHP is not support !");
}

void Judge::copyPerlRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Perl is not support !");
}

void Judge::copyMonoRuntime(const std::string &workdir) {
    throw ClientNotImplementException("C# is not support !");
}

void Judge::copyObjcRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Objc is not support !");
}

void Judge::copyFreebasicRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Basic is not support !");
}

void Judge::copyGuileRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Scheme is not support !");
}

void Judge::copyLuaRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Lua is not support !");
}

void Judge::copyJsRuntime(const std::string &workdir) {
    throw ClientNotImplementException("Javascript is not support !");
}

void Judge::initSafeSysCall(int lang) {
    //reset vector value
    for(std::vector<int>::iterator itr = this->call_counter.begin(); itr != this->call_counter.end(); itr++){
        *itr = 0;
    }

    //set the call_counter
    int i;
    if (lang <= 1||lang==13||lang==14) { // C & C++
        for (i = 0; i==0||LANG_CV[i]; i++) {
            this->call_counter[LANG_CV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 2) { // Pascal
        for (i = 0; i==0||LANG_PV[i]; i++){
            this->call_counter[LANG_PV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 3) { // Java
        for (i = 0; i==0||LANG_JV[i]; i++){
            this->call_counter[LANG_JV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 4) { // Ruby
        for (i = 0; i==0||LANG_RV[i]; i++){
            this->call_counter[LANG_RV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 5) { // Bash
        for (i = 0; i==0||LANG_BV[i]; i++){
            this->call_counter[LANG_BV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 6) { // Python
        for (i = 0; i==0||LANG_YV[i]; i++){
            this->call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 7) { // php
        for (i = 0; i==0||LANG_PHV[i]; i++){
            this->call_counter[LANG_PHV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 8) { // perl
        for (i = 0; i==0||LANG_PLV[i]; i++){
            this->call_counter[LANG_PLV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 9) { // mono c#
        for (i = 0; i==0||LANG_CSV[i]; i++){
            this->call_counter[LANG_CSV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 10) { //objective c
        for (i = 0; i==0||LANG_OV[i]; i++){
            this->call_counter[LANG_OV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 11) { //free basic
        for (i = 0; i==0||LANG_BASICV[i]; i++){
            this->call_counter[LANG_BASICV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 12) { //scheme guile
        for (i = 0; i==0||LANG_SV[i]; i++){
            this->call_counter[LANG_SV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 15) { //lua
        for (i = 0; i==0||LANG_LUAV[i]; i++){
            this->call_counter[LANG_LUAV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 16) { //nodejs
        for (i = 0; i==0||LANG_JSV[i]; i++){
            this->call_counter[LANG_JSV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == 17) { //go
        for (i = 0; i==0||LANG_GOV[i]; i++){
            this->call_counter[LANG_GOV[i]] = HOJ_MAX_LIMIT;
        }
    }

}

std::vector<std::string> Judge::getTestFileListFromLocal(int problem) {
    std::vector<std::string> test_file_list;
    boost::filesystem::path dir_path(this->judge_data_path);    //data path
    dir_path /= to_string(problem);                                 //problem path

    if(!boost::filesystem::exists(dir_path)){
        std::stringstream ss;
        ss << boost::format("Directory %s not exist, and exception happend in Judge::getTestFileListFromLocal") % dir_path.string() << "";
        throw ClientFileException(ss.str());
    }

    boost::filesystem::directory_iterator dir_itr(dir_path);
    boost::filesystem::directory_iterator end;
    for(;dir_itr!=end;dir_itr++){
        const boost::filesystem::path &file_path = dir_itr->path();
        if(file_path.extension()==".in"){
            string test_file_name=file_path.filename().string();
            test_file_list.push_back(test_file_name);
        }
    }
    return test_file_list;
}

void Judge::prepareTestFile(int task, int problem, const std::string &test_file_name) {
    std::stringstream ss;
    //copy the test file to dir
    boost::filesystem::path test_file_source_path(this->judge_data_path);
    test_file_source_path /= to_string(problem);
    test_file_source_path /= test_file_name;                                    // the path string is in this format /home/judge/data/1000/data.in

    boost::filesystem::path test_file_dist_path(this->judge_work_path);
    test_file_dist_path /= "data.in";


    if(!boost::filesystem::exists(test_file_source_path)){
        ss.str("");
        ss << boost::format("Test file %s not exist ! Exception happend in Judge::prepareTestFile") % test_file_source_path.string() << "";
        throw ClientFileException(ss.str());
    }

    //clear the target file if exist
    if(boost::filesystem::exists(test_file_dist_path)){
        boost::filesystem::remove(test_file_dist_path);
    }

    // copy the file and catch exception
    try{
        boost::filesystem::copy_file(test_file_source_path,test_file_dist_path);
    }catch (...){
        ss.str("");
        ss << boost::format("Copy file error from %s to %s in Judge::prepareTestFile") % test_file_source_path.string() % test_file_dist_path.string() << "";
        throw ClientFileException(ss.str());
    }
}

void Judge::runSolution(int task, int lang, int time_limit, int memory_limit, int used_time) {   //Child process
    nice(19);                                                       //lowest running priority
    chdir(this->judge_work_path.c_str());

    freopen("data.in", "r", stdin);
    freopen("user.out", "w", stdout);
    freopen("error.out", "a+", stderr);

    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    if(lang!=3){                                                        //not java
        chroot(this->judge_work_path.c_str());
    }

    //switch to judge user and begin to run the program
    while (setgid(1536) != 0)
        sleep(1);
    while (setuid(1536) != 0)
        sleep(1);
    while (setresuid(1536, 1536, 1536) != 0)
        sleep(1);

    // resource limit struct define
    struct rlimit LIM;

    //cpu time limit
    LIM.rlim_cur = (time_limit - used_time / 1000) + 1;
    LIM.rlim_max = LIM.rlim_cur;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    alarm(time_limit * 10);                                  //finished in 10 second

    //file limit
    LIM.rlim_max = STD_F_LIM + STD_MB;
    LIM.rlim_cur = STD_F_LIM;
    setrlimit(RLIMIT_FSIZE, &LIM);

    //proc limit
    switch (lang) {
        case 17:
            LIM.rlim_cur = LIM.rlim_max = 280;
            break;
        case 3:  //java
        case 4:  //ruby
            //case 6:  //python
        case 9: //C#
        case 12:
        case 16:
            LIM.rlim_cur = LIM.rlim_max = 80;
            break;
        case 5: //bash
            LIM.rlim_cur = LIM.rlim_max = 3;
            break;
        default:
            LIM.rlim_cur = LIM.rlim_max = 1;
    }
    setrlimit(RLIMIT_NPROC, &LIM);

    // stack limit
    LIM.rlim_cur = STD_MB << 6;
    LIM.rlim_max = STD_MB << 6;
    setrlimit(RLIMIT_STACK, &LIM);

    // memory limit
    LIM.rlim_cur = STD_MB * memory_limit / 2 * 3;
    LIM.rlim_max = STD_MB * memory_limit * 2;
    if (lang < 3){
        setrlimit(RLIMIT_AS, &LIM);
    }

    //run the program
    std::stringstream ss;
    switch (lang) {
        case 0:
        case 1:
        case 2:
        case 10:
        case 11:
        case 13:
        case 14:
        case 17:
            this->executeExecvp("./Main");
            break;
        case 3:
            ss.str("");
            ss << boost::format("/usr/bin/java -Xmx%dM %s -Djava.security.manager -Djava.security.policy=./java.policy Main") % memory_limit % this->settings->JAVA_XMS <<"";
            //ss << boost::format("/usr/bin/java /usr/bin/java -Xmx%dM -Djava.security.manager -Djava.security.policy=./java.policy Main") % memory_limit <<"";
            ClientLogger::ERROR(ss.str());
            this->executeExecvp(ss.str());
            break;
        case 4:
            //system("/ruby Main.rb<data.in");
            this->executeExecvp("./ruby Main.rb");
            break;
        case 5: //bash
            this->executeExecvp("/bin/bash Main.sh");
            //execl("/bin/bash", "/bin/bash", "Main.sh", (char *) NULL);
            break;
        case 6: //Python
            this->executeExecvp("/python3 Main.py");
            break;
        case 7: //php
            this->executeExecvp("/php Main.php");
            break;
        case 8: //perl
            this->executeExecvp("/perl Main.pl");
            break;
        case 9: //Mono C#
            this->executeExecvp("/mono --debug Main.exe");
            break;
        case 12: //guile
            this->executeExecvp("/guile Main.scm");
            break;
        case 15: //guile
            this->executeExecvp("/lua Main");
            break;
        case 16: //Node.js
            this->executeExecvp("/nodejs Main.js");
            break;

    }
    //sleep(1);
    fflush(stderr);
    fflush(stdout);
    exit(0);
}

void Judge::watchSolution(pid_t process_id, int task, int lang, int &ac_flag, int &memory_peak, int memory_limit, int &used_time,
                          int time_limit, const std::string &test_file_name) {
    int temp_memory_peak;
    int process_status, process_signal, process_exit_code;
    struct user_regs_struct process_regs;
    struct rusage process_resource_usage;

    boost::filesystem::path out_file_path(this->judge_work_path);
    boost::filesystem::path user_file_path(this->judge_work_path);
    boost::filesystem::path error_file_path(this->judge_work_path);
    out_file_path /= test_file_name;                                        // /home/judge/runX/data.out
    user_file_path /= "user.out";                                           // /home/judge/runX/user.out
    error_file_path /= "error.out";                                         // /home/judge/runX/error.out

    if(temp_memory_peak==0){
        temp_memory_peak = this->getProcessStatus(process_id, "VmRSS:") << 10;
    }

    while(true){
        wait4(process_id, &process_status, 0, &process_resource_usage);

        //lang use vm will ask many memory before need
        if (lang == 3 || lang == 7 || lang == 16 || lang==9 ||lang==17) {
            temp_memory_peak = this->getPageFaultMemory(process_resource_usage);
        } else {        //other use VmPeak
            temp_memory_peak = this->getProcessStatus(process_id, "VmPeak:") << 10;
        }

        if(temp_memory_peak > memory_peak){
            memory_peak = temp_memory_peak;
        }
        if(memory_peak > memory_limit*STD_MB){
            if(ac_flag==JudgeResult::AC){
                ac_flag = JudgeResult::ML;
            }
            ptrace(PTRACE_KILL, process_id, NULL, NULL);            //process need to many memory
            break;
        }

        if (WIFEXITED(process_status)){
            break;
        }

        if ((lang < 4 || lang == 9) && this->getFileSize(error_file_path.string()) ) {
            ac_flag = JudgeResult::RE;
            //std::cout<<"Test Re here"<<std::endl;
            ptrace(PTRACE_KILL, process_id, NULL, NULL);
            break;
        }

        if(this->getFileSize(user_file_path.string()) > this->getFileSize(out_file_path.string())*2+1024 ){
            ac_flag = JudgeResult::OL;
            ptrace(PTRACE_KILL, process_id, NULL, NULL);
        }

        process_exit_code = WEXITSTATUS(process_status);

        /*exitcode == 5 waiting for next CPU allocation          * ruby using system to run,exit 17 ok */
        if ((lang >= 3 && process_exit_code == 17) || process_exit_code == 0x05 || process_exit_code == 0){
            //do nothing and continue to check
        }else{
            if (ac_flag == JudgeResult::AC) {
                switch (process_exit_code) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        ac_flag = JudgeResult::TL;
                        break;
                    case SIGXFSZ:
                        ac_flag = JudgeResult::OL;
                        break;
                    default:
                        ac_flag = JudgeResult::RE;
                }
                //print_runtimeerror(strsignal(exitcode));
            }
            ptrace(PTRACE_KILL, process_id, NULL, NULL);
            break;
        }

        // signal check
        if (WIFSIGNALED(process_status)) {
            /*  WIFSIGNALED: if the process is terminated by signal
             *
             *  psignal(int sig, char *s)，like perror(char *s)，print out s, with error msg from system of sig
             * sig = 5 means Trace/breakpoint trap
             * sig = 11 means Segmentation fault
             * sig = 25 means File size limit exceeded
             */
            process_signal = WTERMSIG(process_status);
            if (ac_flag == JudgeResult::AC) {
                switch (process_signal) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        ac_flag = JudgeResult::TL;
                        break;
                    case SIGXFSZ:
                        ac_flag = JudgeResult::OL;
                        break;
                    default:
                        ac_flag = JudgeResult::RE;
                }
                // print_runtimeerror(strsignal(process_signal));
            }
            break;
        }

        ptrace(PTRACE_GETREGS, process_id, NULL, &process_regs);
        if (call_counter[process_regs.REG_SYSCALL] ){
            //call_counter[reg.REG_SYSCALL]--;
        }else { //do not limit JVM syscall for using different JVM
            ac_flag = JudgeResult::RE;
            ptrace(PTRACE_KILL, process_id, NULL, NULL);
        }
        ptrace(PTRACE_SYSCALL, process_id, NULL, NULL);
    }
    used_time += (process_resource_usage.ru_utime.tv_sec * 1000 + process_resource_usage.ru_utime.tv_usec / 1000);
    used_time += (process_resource_usage.ru_stime.tv_sec * 1000 + process_resource_usage.ru_stime.tv_usec / 1000);
}

int Judge::getProcessStatus(pid_t process_id, std::string key_name) {  //key_name is key name in process status file
    std::stringstream ss;

    FILE * process_status_file;
    int buff_size = 5120;                                         //large value , change the program later
    char buff[buff_size];
    int ret = 0;
    const char *mask = key_name.c_str();

    ss << boost::format("/proc/%d/status") % process_id << "";
    process_status_file = fopen(ss.str().c_str(), "re");

    int m = strlen(mask);
    while (process_status_file && fgets(buff, buff_size - 1, process_status_file)) {

        buff[strlen(buff) - 1] = 0;
        if (strncmp(buff, mask, m) == 0) {
            sscanf(buff + m + 1, "%d", &ret);
        }
    }
    if (process_status_file)
        fclose(process_status_file);
    return ret;
}

int Judge::getPageFaultMemory(struct rusage &resource_usage) {
    //int m_vmpeak, m_vmdata, m_minflt;
    int m_minflt;
    m_minflt = resource_usage.ru_minflt * getpagesize();
    return m_minflt;
}

void Judge::judgeSolution(int &ac_flag, int problem, int lang, int &memory_peak, int memory_limit, int &used_time, int time_limit, const std::string &test_file_name) {
    // number of test not use
    boost::filesystem::path user_file_path(this->judge_work_path);
    boost::filesystem::path out_file_path(this->judge_data_path);
    user_file_path /= "user.out";                                                    // /home/judge/runX/user.out
    out_file_path /= to_string(problem);
    out_file_path /= test_file_name;
    out_file_path.replace_extension("out");                           //  /home/judge/data/100X/test0.out

    ClientLogger::DEBUG("Compare \""+user_file_path.string() +"\" and \""+out_file_path.string() +"\" in Judge:judgeSolution");
    //std::cout<<"Before Current flags:"<<JudgeResult::getStatusDescribeById(ac_flag)<<endl;
    bool compare_result= false;
    if( (ac_flag==JudgeResult::AC)&&(used_time>time_limit*1000) ){
        ac_flag = JudgeResult::TL;
    }
    if( memory_peak> memory_limit*STD_MB){
        ac_flag = JudgeResult::ML;
    }
    if(ac_flag==JudgeResult::AC){
        compare_result = this->compareFile(user_file_path.string(),out_file_path.string());
        if(compare_result== false){
            ac_flag = JudgeResult::WA;
        }
    }
    if(lang==3){
        this->fixJavaMistakeJudge(ac_flag, memory_peak, memory_limit);
    }
    if(lang==6){
        this->fixPythonMistakeJudge(ac_flag,memory_peak,memory_limit);
        //fix python miss judge
    }

    // Debug only
    if(compare_result== false){
        ClientLogger::DEBUG("Two file \""+user_file_path.string() +"\" and \""+out_file_path.string() +"\" is different Judge:judgeSolution" );
    }else{
        ClientLogger::DEBUG("Two file \""+user_file_path.string() +"\" and \""+out_file_path.string() +"\" have same content Judge:judgeSolution" );
    }
    //std::cout<<"After Current flags:"<<JudgeResult::getStatusDescribeById(ac_flag)<<endl;
}

void Judge::fixJavaMistakeJudge(int &ac_flag, int &memory_peak, int memory_limit) {
    std::ifstream source_stream;
    std::stringstream ss;

    try{
        ss.str("");
        ss << boost::format("%s/error.out") % this->judge_work_path << "";
        boost::filesystem::path error_out_path(ss.str());

        if(!boost::filesystem::exists(error_out_path)){     //if error.out not exist ,just return
            return;
        }

        // error.out exist origin code
        /*
        ss.str("");
        ss << boost::format("chmod 700 %s/error.out") % this->judge_work_path << "";
        this->executeSystemCommand(ss.str());
        */
        chmod(error_out_path.string().c_str(),S_IRUSR|S_IWUSR|S_IXUSR );

        //read the file content
        source_stream.open(error_out_path.string());
        ss.str("");
        ss << source_stream.rdbuf();
        string source_file_content = ss.str();
        source_stream.close();

        bool search_result;
        search_result = boost::algorithm::contains(source_file_content,"java.lang.OutOfMemoryError");
        // might have bug
        if(search_result== true){
            ac_flag = JudgeResult::ML;
            memory_peak = memory_limit* STD_MB;
        }

        search_result = boost::algorithm::contains(source_file_content,"Could not create");
        if(search_result== true){
            ac_flag = JudgeResult::RE;
        }
    }catch (...){
        source_stream.close();
        ClientLogger::ERROR("Exception in Judge::fixJavaMistakeJudge !");
    }

}

void Judge::fixPythonMistakeJudge(int &ac_flag, int &memory_peak, int memory_limit) {
    std::ifstream source_stream;
    std::stringstream ss;

    try{
        ss.str("");
        ss << boost::format("%s/error.out") % this->judge_work_path << "";
        boost::filesystem::path error_out_path(ss.str());

        if(!boost::filesystem::exists(error_out_path)){     //if error.out not exist ,just return
            return;
        }

        //read the file content
        source_stream.open(error_out_path.string());
        ss.str("");
        ss << source_stream.rdbuf();
        string source_file_content = ss.str();
        source_stream.close();

        bool search_result;
        search_result = boost::algorithm::contains(source_file_content,"MemoryError");
        if(search_result== true){
            ac_flag = JudgeResult::ML;
            memory_peak = memory_limit * STD_MB;
        }
    }catch (...){
        source_stream.close();
        ClientLogger::ERROR("Exception in Judge::fixPythonMistakeJudge !");
    }
}














