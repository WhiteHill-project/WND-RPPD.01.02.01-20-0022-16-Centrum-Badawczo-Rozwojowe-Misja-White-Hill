cmake_minimum_required(VERSION 3.0)
project(Triffid_1)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DWIN32 -DNOMINMAX -D_WIN32_WINNT=0x501)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wno-unused-variable -Wno-reorder -std=c++11" )
ENDIF()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH})
set(ThirdParty_Install_Dir ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/built)

add_subdirectory(3rd_party)

INCLUDE_DIRECTORIES(${ThirdParty_Install_Dir}/include)
enable_testing()

add_subdirectory(DataSetWorker)

add_subdirectory(TestServerHarness)

add_subdirectory(DP_Modules)
