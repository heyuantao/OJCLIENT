//
// Created by he_yu on 2020-01-25.
//

#ifndef OJCLIENT_LOGGER_H
#define OJCLIENT_LOGGER_H

#include <iostream>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

class ClientLogger{
public:
    static string file_name;
    static std::stringstream string_stream;
    static bool debug;
    //static ofstream output_stream;
    //static bool tostdout;
public:
    static void DEBUG(std::string msg);
    static void WARN(std::string msg);
    static void ERROR(std::string msg);

};

#endif //OJCLIENT_LOGGER_H
