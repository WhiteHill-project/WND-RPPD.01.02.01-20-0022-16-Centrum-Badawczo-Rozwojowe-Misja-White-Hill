set(dsw_SRCS
    src/connection.h
    src/connection.cpp
)
INCLUDE_DIRECTORIES(src)

add_library(DataSetWorker STATIC ${dsw_SRCS})
set_property(TARGET DataSetWorker APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(TestSetWorker)

add_subdirectory(tests)
