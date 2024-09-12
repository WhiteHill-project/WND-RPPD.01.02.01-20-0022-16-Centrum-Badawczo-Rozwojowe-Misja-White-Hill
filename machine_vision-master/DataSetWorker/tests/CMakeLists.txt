find_package(Qt5Test REQUIRED)

add_subdirectory(mocks)

add_executable(connectionTest connectionTest.cpp)
target_link_libraries(connectionTest DataSetWorker ConnectionMock gtest_IMP)
