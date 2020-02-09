#include <iostream>
#include <vector>

#include "Rest/Client.h"
#include "Common/Exceptions.h"
#include "Common/Logger.h"
#include "Core/Daemon.h"

using namespace std;

bool checkArg(int argc, char** argv, bool &daemon, bool &debug){
    std::string daemon_string="";
    std::string debug_string="";
    bool daemon_status = false;
    bool debug_status = false;

    if(argc>3){
        ClientLogger::ERROR("arg is not set properly !");
        exit(1);
    }
    if(argc==2){
        daemon_string = argv[1];
    }
    if(argc==3){
        daemon_string = argv[1];
        debug_string = argv[2];
    }

    if(boost::to_lower_copy(daemon_string)=="nodaemon"){
        daemon_status = false;
    }else{
        daemon_status = true;
    }

    if(boost::to_lower_copy(debug_string)=="debug"){
        debug_status = true;
    }else{
        debug_status = false;
    }

    daemon = daemon_status;
    debug = debug_status;
}

int main(int argc, char** argv) {
    bool daemon= true;
    bool debug = false;
    int status;
    status = checkArg(argc, argv, daemon, debug);

    try {
        Daemon job("/home/judge/", daemon, debug);
        job.init();
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

