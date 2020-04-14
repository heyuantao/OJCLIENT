//
// Created by he_yu on 2020-01-25.
//

#ifndef OJCLIENT_EXCEPTIONS_H
#define OJCLIENT_EXCEPTIONS_H

#include <iostream>
#include <exception>

using namespace std;

class ClientException{
public:
    virtual void printException(){};
    virtual ~ClientException(){} ;
protected:
    std::string error_message;
    static bool debug;
};

class ClientFileException : public ClientException{
public:
    ClientFileException(std::string msg);
    virtual void printException();
};

class ClientMessageException : public ClientException{
public:
    ClientMessageException(std::string msg);
    virtual void printException();
};

class ClientNetworkException :  public ClientException{
public:
    ClientNetworkException(std::string msg) ;
    virtual void printException() ;
};

class ClientUnauthorizedException :  public ClientException{
public:
    ClientUnauthorizedException(std::string msg) ;
    virtual void printException() ;
};

class ClientRunCommandException : public ClientException{
public:
    ClientRunCommandException(std::string msg);
    virtual void printException() ;
};

class ClientContainerException : public ClientException {
public:
    ClientContainerException(std::string msg);
    virtual void printException() ;
};

class ClientNotImplementException: public ClientException{
public:
    ClientNotImplementException(std::string msg);
    virtual void printException();
};

class ClientAppException : public ClientException {
public:
    ClientAppException(std::string msg);
    virtual void printException() ;
};

#endif //OJCLIENT_EXCEPTIONS_H
