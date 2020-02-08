//
// Created by he_yu on 2020-01-27.
//

#include <iostream>

#include <boost/filesystem.hpp>

#include "Rest/Client.h"
#include "Common/Exceptions.h"
#include "Common/Logger.h"
#include "Core/Judge.h"

using namespace std;

bool checkArg(int argc, char** argv, string &solution, string &task){
    if(argc<3){
        ClientLogger::ERROR("arg is not set properly !");
        exit(1);
    }
    solution = argv[1];
    task = argv[2];
}

int main(int argc, char** argv) {
    int status;
    std::string solution,task;
    status = checkArg(argc, argv, solution, task);

    try{
        Judge judge("/home/judge", solution, task);
        judge.init();
        judge.run();
    }catch (ClientException &e){
        e.printException();
    }catch (...){
        std::cout<<"Unknow error happen !"<<std::endl;
    }
    return 0;
}