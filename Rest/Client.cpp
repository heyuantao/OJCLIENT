//
// Created by he_yu on 2020-01-21.
//
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "Client.h"
#include "Rest.h"
#include "../Common/Utils.h"
#include "../Common/Exceptions.h"
#include "../Common/Logger.h"

using namespace std;

std::vector<std::string> const Client::language_extension = { "c", "cc", "pas", "java", "rb", "sh", "py", "php", "pl", "cs", "m", "bas", "scm","c","cc","lua","js","go" };

Client::Client(const std::string &base_url){
};

std::vector<std::string> Client::getJobs(){
}

//result = 0 mean is judge,4 is success status
void Client::updateSolution(int solution=1001, int result=0, int time=0, int memory=0, double pass_rate=0.0){
}

void Client::addCompileErrorInformation(int solution) {
}

void Client::getSolution(int solution=1000, std::string work_dir="/tmp", int language=0) {
}

void Client::getSolutionInformation(int solution, int &problem, std::string &username, int &lang) {
}

void Client::getProblemInformation(int problem, int &time_limit, int &mem_limit, int &isspj) {
}

void Client::addRuningErrorInformation(int solution) {
}

void Client::getTestFile(int problem, std::string data_dir) {
}

string Client::getLanguageExtensionById(int lang) {
}

void Client::updateUserInformation(const std::string username) {
}

void Client::login(const std::string &token) {
}

bool Client::checkLogin() {
}


