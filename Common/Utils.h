//
// Created by he_yu on 2020-01-22.
//

#ifndef OJCLIENT_UTILS_H
#define OJCLIENT_UTILS_H

#include <map>
#include <vector>
#include <iostream>
#include <string>

#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>



using namespace std;

/*
class ClientFile{
private:
    string filename;
    struct stat filestat;
public:
    ClientFile(string filename);
    string getModifyTime();
    bool exist();
};

class ClientPath{
public:
    static string join(string dir_name,string file_name);
};
 */


class ClientSettings{
private:
    std::string config_file_path;
public:
    uid_t process_uid;               //uid and gid is use for process to set directory and file permission /home/judge/runx
    gid_t process_gid;
public:
    std::string BASEURL;
    std::string MAX_RUNINING;
    std::string SLEEP_TIME;
    std::string LANG_SET;
    std::string JAVA_TIME_BONUS;
    std::string JAVA_MEMORY_BONUS;
    std::string JAVA_XMS;
    std::string JAVA_XMX;
    //string COMPILE_CHROOT;
    std::string API_KEY;        //use in http header for api permission
public:
    ClientSettings(const std::string &config_file_path="/home/judge/etc/judge.conf");
    void init();
private:
    std::map<string,string> fileContentToKeyValue();
    bool parseOneLineToKeyAndValue(std::string line,std::string &key,std::string &value);
};

#endif //OJCLIENT_UTILS_H