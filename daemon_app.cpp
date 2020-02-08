#include <iostream>
#include <vector>

#include "Rest/Client.h"
#include "Common/Exceptions.h"
#include "Common/Logger.h"
#include "Core/Daemon.h"

using namespace std;

bool checkArg(int argc, char** argv, bool &daemon){
    if(argc>2){
        ClientLogger::ERROR("arg is not set properly !");
        exit(1);
    }
    if(argc==2){
        daemon= false;
    }
}

int main(int argc, char** argv) {
    bool daemon= true;
    int status;
    status = checkArg(argc, argv, daemon);

    try {
        Daemon job("/home/judge/",daemon);
        job.run();
    } catch (ClientException &e) {
        e.printException();
    }catch (...){
        std::cout<<"Unknow error happen !"<<std::endl;
    }
    return 0;
}


    //Daemon daemon();
    //cout<<ClientPath::join("","");
    //test getjob()
    /*
    vector<string> id_list=client.getJobs();
    vector<string>::iterator itr;
    cout<<"Output!"<<endl;
    for(itr=id_list.begin();itr!=id_list.end();itr++){
        cout<<*itr<<endl;
    }*/

    //client.updateSolution(1029,0,0,0,0);
    //client.updateSolution(1028,0,0,0,0);
    //client.updateSolution(1027,0,0,0,0);
    //client.updateSolution(1026,0,0,0,0);
    //client.getSolution(1002,"./data",0);

    //test for getSolutionInformation
    /*
    int problem;
    string username;
    int lang;
    client.getSolutionInformation(1002,problem,username,lang);
    std::cout<<problem<<username<<lang;
    cout<<"finished";
     */

    //test for getProblemInformation
    /*
    int time_limit, mem_limit, isspj;
    client.getProblemInformation(1001,time_limit, mem_limit, isspj);
    cout<<time_limit<<":"<<mem_limit<<":"<<isspj<<endl;
     */

    //test for addceinfo
    //client.addCompileErrorInformation(1030);

    //test getTestFile()
    //client.getTestFile(1006,"./data/");

