#pragma once
class DataSet;

class WorkHandler
{
public:
    virtual void connectionEstablished()  = 0;
    virtual void connectionLost()         = 0;
    virtual void dataAvailable(DataSet *) = 0;
};

class WorkerConnection
{
public:
    virtual ~WorkerConnection() {}
    virtual void registerHandler(WorkHandler *wrk) = 0;
    virtual void close()                           = 0;
    virtual int  processEvents() = 0;
    virtual void dataSetProcessed(DataSet *) = 0;
};

class ServerConnector
{
public:
    virtual ~ServerConnector() {}
    virtual WorkerConnection *establishConnection(const char *server_addr) = 0;
    virtual void closeConnection(WorkerConnection *)                       = 0;
};
