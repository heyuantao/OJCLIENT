//
// Created by he_yu on 2020-01-22.
//
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Utils.h"
#include "Exceptions.h"
#include "Logger.h"

using namespace std;

/*
ClientFile::ClientFile(string filename) {
    this->filename = filename;
}

string ClientFile::getModifyTime() {
    stat(this->filename.c_str(), &this->filestat);
    time_t time= this->filestat.st_mtime;
    return to_string(time) ;
}

bool ClientFile::exist() {
    if(access(this->filename.c_str(),F_OK)==0){
        return true;
    }else{
        return false;
    }
}

string ClientPath::join(string dir_name, string file_name) {
    if(file_name.size()==0){
        return dir_name;
    }
    if(dir_name.size()==0){
        return file_name;
    }
    //file_name and dir_name is not empty

    int dir_name_length = dir_name.size();
    char dir_name_last_char = dir_name[dir_name_length-1];
    char file_name_first_char = file_name[0];

    stringstream ss;
    if( (dir_name_last_char=='/') &&(file_name_first_char=='/')){
        ss << dir_name.substr(0,dir_name_length-1) << file_name ;
    } else if ( (dir_name_last_char != '/') && (file_name_first_char=='/') ){
        ss << dir_name << file_name ;
    } else if( (dir_name_last_char == '/') && (file_name_first_char!='/') ){
        ss << dir_name << file_name ;
    } else{
        ss << dir_name << '/' << file_name ;
    }
    return ss.str();
}
 */

ClientSettings::ClientSettings(const std::string &config_file_path) {
    this->config_file_path = config_file_path;
    //workdir permission set
    this->process_uid = 1536;
    this->process_gid = 1536;
    //api auth
    this->BASEURL = "http://localhost";
    this->API_KEY = "";
    //other settings
    this->MAX_RUNINING = "2";
    this->SLEEP_TIME = "1";
    this->LANG_SET = "1,2,3";
    this->JAVA_TIME_BONUS = "5";
    this->JAVA_MEMORY_BONUS = "512";
    this->JAVA_XMS = "-Xms64m";
    this->JAVA_XMX = "-Xmx128m";
    //this->COMPILE_CHROOT = "true";
}

std::map<std::string,std::string> ClientSettings::fileContentToKeyValue(){
    std::stringstream ss;

    std::map<std::string,std::string> config;
    std::string file_content="";
    //ClientFile setting_file(this->config_file_path);
    boost::filesystem::path setting_file_path(this->config_file_path);

    if(!boost::filesystem::exists(setting_file_path)){
    //if(setting_file.exist()== false){
        ss.str();
        ss << boost::format("Configurae file \"%s\"not exist , Using default settings !") % this->config_file_path << "";
        throw ClientFileException(ss.str());
    }
    try{
        std::ifstream input_stream;
        std::string one_line;
        std::string key="";
        std::string value="";

        input_stream.open(this->config_file_path.c_str());
        while(input_stream>>one_line){
            parseOneLineToKeyAndValue(one_line,key,value);
            config.insert(pair<std::string,std::string>(key,value));
        }
        input_stream.close();
        return config;
    }catch(...){
        throw ClientFileException("Read file and parseFile() error:"+this->config_file_path);
        return config;
    }
}

bool ClientSettings::parseOneLineToKeyAndValue(std::string line, std::string &key, std::string &value) {
    std::vector<std::string> string_list;
    boost::split(string_list,line,boost::is_any_of("="));
    if(string_list.size()!=2){
        key = "";
        value = "";
    }else{
        key = string_list[0];
        value = string_list[1];
        boost::trim(key);
        boost::trim(value);
    }
}

void ClientSettings::init() {
    try{
        boost::filesystem::path c_path(this->config_file_path);
        if(!boost::filesystem::exists(c_path)){
            std::stringstream ss;
            ss << boost::format("Configuration file \"%s\" not exist , Using default settings !") % c_path.string() << "";
            throw ClientFileException(ss.str());
        }

        //this->config_file_path = config_file_path;
        std::map<std::string,std::string> config_key_value = this->fileContentToKeyValue();
        std::map<std::string,std::string>::iterator itr;
        if((itr = config_key_value.find("BASEURL"))!=config_key_value.end()){
            this->BASEURL = itr->second;
        }
        if((itr = config_key_value.find("API_KEY"))!=config_key_value.end()){
            this->API_KEY = itr->second;
        }
        if((itr = config_key_value.find("MAX_RUNINING"))!=config_key_value.end()){
            this->MAX_RUNINING = itr->second;
        }
        if((itr = config_key_value.find("SLEEP_TIME"))!=config_key_value.end()){
            this->SLEEP_TIME = itr->second;
        }
        if((itr = config_key_value.find("LANG_SET"))!=config_key_value.end()){
            this->LANG_SET = itr->second;
        }
        if((itr = config_key_value.find("JAVA_TIME_BONUS"))!=config_key_value.end()){
            this->JAVA_TIME_BONUS = itr->second;
        }
        if((itr = config_key_value.find("JAVA_MEMORY_BONUS"))!=config_key_value.end()){
            this->JAVA_MEMORY_BONUS = itr->second;
        }
        if((itr = config_key_value.find("JAVA_XMS"))!=config_key_value.end()){
            this->JAVA_XMS = itr->second;
        }
        if((itr = config_key_value.find("JAVA_XMX"))!=config_key_value.end()){
            this->JAVA_XMX = itr->second;
        }

    }catch (ClientException &e){
        e.printException();
    }
}

