#include "gtest/gtest.h"
#include "mocks/mock_connection.h"


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
