#ifndef APISERVER_H
#define APISERVER_H

#include "../QueryProcessing/QueryProcessor.h"

class ApiServer {
private:
    QueryProcessor queryProcessor;

public:
    ApiServer();
    void start(int port);
};

#endif