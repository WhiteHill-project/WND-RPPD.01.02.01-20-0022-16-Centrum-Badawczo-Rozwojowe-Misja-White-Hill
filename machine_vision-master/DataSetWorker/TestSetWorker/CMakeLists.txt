add_library(TestSetConnector test_connector.cpp)
target_link_libraries(TestSetConnector DataSetWorker)

set_property(TARGET TestSetConnector APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

