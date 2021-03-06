//
// Created by he_yu on 2020-01-25.
//

#ifndef OJCLIENT_DAEMON_H
#define OJCLIENT_DAEMON_H
#include <iostream>
#include <string>

#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/resource.h>

#include "../Common/Utils.h"
#include "../Rest/Client.h"

using namespace std;

class Daemon{
private:
    bool daemon;                    //if run in daemon mode ,if not run in daemon the process will print debug message
    //bool debug;                     //if run in debug mode,with will display debug message and output to log file
    std::string daemon_home_path;
    std::string lock_file_name;
    std::string lock_file_path;
    ClientSettings *settings;        //setting from judge.conf
    Client *client;                  //client instance for web
public:
    Daemon(const std::string &daemon_home_path="/home/judge/",bool daemon=true);
    void init();
    ~Daemon();
    bool run();
private:
    bool becameDaemonProcess();
    bool alreadyRunning() ;
    bool work();
    bool runJudge(std::string solution,int task);
};

#endif //OJCLIENT_DAEMON_H
