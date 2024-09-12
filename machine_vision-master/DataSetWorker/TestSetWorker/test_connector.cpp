#include "test_connector.h"

#include "connection.h"
#include "DataSet.h"
#include <thread>
#include <stdio.h>

struct TestWorkerConnection_Base : public WorkerConnection {

    WorkHandler * m_wrk_callbacks;
    bool m_running;
    // WorkerConnection interface
public:
    void registerHandler(WorkHandler * wrk) override
    {
        m_wrk_callbacks = wrk;
    }
    void close() override
    {
        m_wrk_callbacks->connectionLost();
    }
    int processEvents() override
    {
        m_running = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        if(m_wrk_callbacks) {
            m_wrk_callbacks->connectionEstablished();
        }
        while(m_running) {
            if(m_wrk_callbacks) {
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
                DataSet * ds = getNextDataSet();
                m_wrk_callbacks->dataAvailable(ds);
                dataSetProcessed(ds);
            }
        }
        return 0;
    }
    virtual void dataSetProcessed(DataSet *v) override
    {
        printf("Data set processed.\n");
        delete v;
    }
    virtual DataSet * getNextDataSet() = 0;
};

struct TestWorkerConnection : public TestWorkerConnection_Base {
    DataSet * getNextDataSet() { return nullptr; }
};

struct TestServerConnector : public ServerConnector
{
    // ServerConnector interface
public:
    WorkerConnection *establishConnection(const char * server_addr) override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        return new TestWorkerConnection;

    }
    void closeConnection(WorkerConnection *v) override
    {
        delete v;
    }
};


ServerConnector *createConnectorLevel0()
{
    return new TestServerConnector;
}

ServerConnector *createConnectorLevel1()
{

}
