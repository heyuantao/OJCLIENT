//
// Created by he_yu on 2020-01-27.
//

#include <iostream>

#include "Rest/Client.h"
#include "Common/Exceptions.h"
#include "Common/Logger.h"
#include "Core/Judge.h"

using namespace std;

bool checkArg(int argc, char** argv, string &solution, string &task, string &debug){
    if((argc<3)||(argc>4)){
        ClientLogger::ERROR("arg is not set properly !");
        exit(1);
    }
    solution = argv[1];
    task = argv[2];
    if(argc==4){
        debug= argv[3];
    }else{
        debug= "";
    }
}

int main(int argc, char** argv) {
    int status;
    std::string solution,task,debug_string;
    bool debug;
    status = checkArg(argc, argv, solution, task, debug_string);
    if(boost::to_lower_copy(debug_string)=="debug"){
        debug= true;
    }else{
        debug= false;
    }

    try{
        Judge judge("/home/judge", solution, task, debug);
        judge.init();
        judge.run();
    }catch (ClientException &e){
        e.printException();
    }catch (...){
        std::cout<<"Unknow error happen !"<<std::endl;
    }
    return 0;
}