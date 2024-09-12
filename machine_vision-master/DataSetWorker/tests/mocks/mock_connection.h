#pragma once
#include "connection.h"
#include "gmock/gmock.h"  // Brings in Google Mock.
class MockTurtle : public ServerConnector {
public:
    MOCK_METHOD1(establishConnection, WorkerConnection *(const char * distance));
    MOCK_METHOD1(closeConnection, void(WorkerConnection *cn));
};
