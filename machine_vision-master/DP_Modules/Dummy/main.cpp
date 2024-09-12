#include "DataSet.h"
#include "connection.h"
#include "test_connector.h"
#include <stdio.h>

struct SampleWorkHandler : public WorkHandler
{

    // WorkHandler interface
private:
    void connectionEstablished() override { printf("We are connected !\n"); }
    void connectionLost() override { printf("Connection lost !\n"); }
    void dataAvailable(DataSet *) override { printf("New data to be processed !\n"); }
};

int main(int argc, char **argv)
{
    ServerConnector *connector = createConnectorLevel0();

    WorkerConnection *w_conn = connector->establishConnection("127.0.0.1/Test");
    SampleWorkHandler s_w_h;
    w_conn->registerHandler(&s_w_h);
    return w_conn->processEvents();
}
