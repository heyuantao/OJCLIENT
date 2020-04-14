//
// Created by he_yu on 2020-01-25.
//
#include "Exceptions.h"

using namespace std;

bool ClientException::debug = true;


ClientNetworkException::ClientNetworkException(std::string msg){
    this->error_message = msg;
}

void ClientNetworkException::printException() {
    std::cout << "ClientNetowrkException : " << this->error_message << std::endl;
}

ClientUnauthorizedException::ClientUnauthorizedException(std::string msg){
    this->error_message = msg;
}

void ClientUnauthorizedException::printException() {
    std::cout << "ClientUnauthorizedException : " << this->error_message << std::endl;
}

ClientRunCommandException::ClientRunCommandException(std::string msg) {
    this->error_message = msg;
}

void ClientRunCommandException::printException() {
    std::cout << "ClientRunCommandException : " << this->error_message << std::endl;
}

ClientMessageException::ClientMessageException(std::string msg) {
    this->error_message = msg;
}

void ClientMessageException::printException() {
    std::cout << "ClientRunCommandException : " << this->error_message << std::endl;
}

ClientFileException::ClientFileException(std::string msg) {
    this->error_message = msg;
}

void ClientFileException::printException() {
    std::cout << "ClientFileException : " << this->error_message << std::endl;
}


ClientContainerException::ClientContainerException(std::string msg) {
    this->error_message = msg;
}

void ClientContainerException::printException() {
    std::cout << "ClientContainerException : " << this->error_message << std::endl;
}

ClientAppException::ClientAppException(std::string msg) {
    this->error_message = msg;
}

void ClientAppException::printException() {
    std::cout << "ClientAppException : " << this->error_message << std::endl;
}


ClientNotImplementException::ClientNotImplementException(std::string msg) {
    this->error_message = msg;
}

void ClientNotImplementException::printException() {
    std::cout << "ClientNotImplementException : " << this->error_message << std::endl;
}
