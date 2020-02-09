//
// Created by he_yu on 2020-01-25.
//
#include <algorithm>

#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>

#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

#include "Daemon.h"
#include "../Common/Utils.h"
#include "../Common/Logger.h"
#include "../Common/Exceptions.h"

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)  //for lock file function
#define STD_MB 1048576

using namespace std;

//ClientSettings Daemon::settings

Daemon::Daemon(const std::string &daemon_home_path, bool daemon){
    this->daemon_home_path = daemon_home_path;
    this->daemon = daemon;
    //this->debug = debug;
    this->lock_file_name = "etc/judge.pid";
    boost::filesystem::path lock_file_p(this->daemon_home_path);
    lock_file_p /= this->lock_file_name;
    this->lock_file_path = lock_file_p.string();

    this->settings = NULL;
    this->client = NULL;
}

void Daemon::init(){
    try{
        this->settings = new ClientSettings("/home/judge/etc/judge.conf");
        this->settings->init();  //string config_file_path = ClientPath::join(this->daemon_home_path,"etc/judge.conf");

        //this->client = new Client(this->settings->BASEURL);
        this->client = Client::createClient("HustojClient",this->settings->BASEURL);
    }catch (ClientException &e){
        e.printException();
        std::stringstream ss;
        ss << boost::format("Exception in Daemon::init()") <<"";
        throw ClientMessageException(ss.str());
    }catch (...){
        std::stringstream ss;
        ss << boost::format("Unknow exception in Daemon::init()") <<"";
        throw ClientMessageException(ss.str());
    }
}

Daemon::~Daemon(){
    delete this->client;
    this->client = NULL;
    delete this->settings;
    this->settings = NULL;
}

bool Daemon::becameDaemonProcess() {
    pid_t pid = fork();
    if(pid < 0){            // fork error
        return false;
    }else if(pid != 0){     // parent exit
        exit(0);
    }
    //child process
    setsid();               // become session leader
    chdir(this->daemon_home_path.c_str());
    umask(0);

    close(0);
    close(1);
    close(2);

    int fd = open("/dev/null",O_RDWR);
    dup2(fd,0);
    dup2(fd,1);
    dup2(fd,2);
    if(fd>2){
        close(fd);
    }
    return true;
}

bool Daemon::alreadyRunning() {  //may have bug in status
    int lock_file_fd;
    lock_file_fd = open(this->lock_file_path.c_str(),O_RDWR | O_CREAT, LOCKMODE);
    if(lock_file_fd<0){
        ClientLogger::ERROR("Can not open lock file !");
        exit(1);
    }

    struct flock file_locker;
    file_locker.l_type = F_WRLCK;
    file_locker.l_start = 0;
    file_locker.l_whence = SEEK_SET;
    file_locker.l_len = 0;
    int status = fcntl(lock_file_fd, F_SETLK, &file_locker);
    if(status<0){
        if (errno == EACCES || errno == EAGAIN) {
            close(lock_file_fd);
            return true;
        }
        ClientLogger::ERROR("Can not lock file !");
        exit(1);
    }
    ftruncate(lock_file_fd, 0);
    char buff[16];
    sprintf(buff, "%d", getpid());
    write(lock_file_fd, buff, strlen(buff) + 1);
    return false;
}

bool Daemon::run() {
    ClientLogger::DEBUG("Login and checkLogin in Daemon::run()");
    this->client->login(this->settings->API_KEY);
    if(!this->client->checkLogin()){
        ClientLogger::ERROR("Can not login in Daemon::run()");
        return false;
    };

    chdir(this->daemon_home_path.c_str());

    ClientLogger::DEBUG("Run the program in Daemon::run()");
    if(this->daemon== true){                                                   // not run in daemon mode
        ClientLogger::DEBUG("Switch to Daemon mode in Daemon::run()");
        this->becameDaemonProcess();
    }

    if(this->alreadyRunning()){
        ClientLogger::ERROR("This daemon is already running !");
        return false;
    }

    while(true){
        int sleep_time = std::stoi(this->settings->SLEEP_TIME);
        this->work();
        sleep(sleep_time);
    }
}

bool Daemon::work() {
    std::stringstream ss;
    vector<std::string> job_string_list = this->client->getJobs();

    int max_running = stoi(this->settings->MAX_RUNINING);
    int running = 0;
    std::vector<int> task_list(max_running,0);   //task slot from 0 to (max_running-1),and init with zero value

    std::vector<string>::iterator job_itr;
    std::string job_id ;
    std::vector<int>::iterator find_task_itr;

    ss.str("");
    ss << boost::format("Get %d job in Daemon::work()") % job_string_list.size() << "";
    ClientLogger::DEBUG(ss.str());

    for(job_itr = job_string_list.begin();job_itr!=job_string_list.end();job_itr++){
        job_id = *job_itr;
        if(running>=max_running){        //run the task !
            int finished_task_pid = waitpid(-1,NULL,0);
            find_task_itr = std::find(task_list.begin(),task_list.end(),finished_task_pid);
            if( find_task_itr != task_list.end() ){
                running = running -1;
                *find_task_itr = 0;
            }
        }else{                          //find a empty task to run the task !
            find_task_itr = std::find(task_list.begin(),task_list.end(),0);
        }
        if(find_task_itr!=task_list.end()){
            //submit is going to run the solution id(string) at task solt(find_task_itr)
            int process_id = fork();
            if (process_id > 0){                // parent process
                *find_task_itr = process_id;
                running = running +1;
            } else if (process_id==0){           // child process
                int solution_id = std::stoi(*job_itr);
                int task_id = find_task_itr-task_list.begin();

                this->runJudge(solution_id,task_id);
                exit(0);
            }

        }
    }

    int task_pid;
    while((task_pid = waitpid(-1,NULL,0))>0){
        find_task_itr = std::find(task_list.begin(),task_list.end(),task_pid);
        if(find_task_itr!=task_list.end()){
            *find_task_itr =0;
            running = running -1;
        }
    }

    ss.str("");
    ss << boost::format("Finish %d solutions in Daemon::work()") % running << "";
    ClientLogger::DEBUG(ss.str());

    return true;
}

bool Daemon::runJudge(int solution,int task) {
    std::stringstream ss;
    ss.str("");
    ss << boost::format("Start client with solution %d at solt %d in Daemon::runJudge()") % solution % task << "";
    ClientLogger::DEBUG(ss.str());

    struct rlimit LIM;
    LIM.rlim_max = 800;
    LIM.rlim_cur = 800;
    setrlimit(RLIMIT_CPU, &LIM);

    LIM.rlim_max = 180 * STD_MB;
    LIM.rlim_cur = 180 * STD_MB;
    setrlimit(RLIMIT_FSIZE, &LIM);

    LIM.rlim_max = STD_MB << 11;
    LIM.rlim_cur = STD_MB << 11;
    setrlimit(RLIMIT_AS, &LIM);

    LIM.rlim_cur = LIM.rlim_max = 200;
    setrlimit(RLIMIT_NPROC, &LIM);

    //execl("/usr/bin/judge_client", "/usr/bin/judge_client", std::to_string(solution).c_str(), std::to_string(task).c_str(), this->daemon_home_path.c_str(), (char *) NULL);
    if(this->daemon== true){
        execl("/usr/local/bin/JUDGE", "/usr/local/bin/JUDGE", std::to_string(solution).c_str(), std::to_string(task).c_str(), (char *) NULL);
    }else{
        execl("/usr/local/bin/JUDGE", "/usr/local/bin/JUDGE", std::to_string(solution).c_str(), std::to_string(task).c_str(), "debug", (char *) NULL);
    }
    sleep(5);
    return false;
}
