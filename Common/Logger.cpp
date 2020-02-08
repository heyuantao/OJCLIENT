//
// Created by he_yu on 2020-01-25.
//
#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>

#include "Logger.h"

using namespace std;

//std::string ClientLogger::file_name = "/tmp/judge_log.txt";
std::stringstream ClientLogger::string_stream;
bool ClientLogger::debug= false;

void ClientLogger::DEBUG(std::string log_message){
    ClientLogger::string_stream.str("");
    ClientLogger::string_stream << "DEBUG : " << log_message <<std::endl;
    //syslog(LOG_DEBUG | LOG_DAEMON, "%s", log_message.c_str());
    std::cout << string_stream.str() ;

    /*
    std::ofstream file_logger( ClientLogger::file_name, ios::app);
    file_logger << string_stream.str() ;
    file_logger.close();
     */
}

void ClientLogger::WARN(std::string log_message) {
    ClientLogger::string_stream.str("");
    ClientLogger::string_stream << "WARN : " << log_message <<std::endl;
    //syslog(LOG_WARNING | LOG_DAEMON, "%s", log_message.c_str());
    std::cout << string_stream.str() ;

    /*
    std::ofstream file_logger( ClientLogger::file_name, ios::app);
    file_logger << string_stream.str() ;
    file_logger.close();
     */
}

void ClientLogger::ERROR(std::string log_message) {
    ClientLogger::string_stream.str("");
    ClientLogger::string_stream << "ERROR : " << log_message <<std::endl;
    //syslog(LOG_ERR | LOG_DAEMON, "%s", log_message.c_str());
    std::cout << string_stream.str() ;

    /*
    std::ofstream file_logger( ClientLogger::file_name, ios::app);
    file_logger << string_stream.str() ;
    file_logger.close();
     */
}


