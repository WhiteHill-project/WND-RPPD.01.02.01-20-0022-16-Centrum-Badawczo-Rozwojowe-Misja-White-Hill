cmake_minimum_required(VERSION 3.0)
project(aravis)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC TRUE)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeScripts;${CMAKE_MODULE_PATH})

find_package(Qt5Core 5.7.0 REQUIRED)
find_package(Qt5Widgets 5.7.0 REQUIRED)
find_package(Qt5Network 5.7.0 REQUIRED)
find_package(Qt5Xml 5.7.0 REQUIRED)
find_package(LibZip)


include(3rdparty_support)
set(ThirdParty_Install_Dir ${CMAKE_BINARY_DIR}/3rd_party/built) # ERICEDIT: Changed to CMAKE_BINARY_DIR so that this variable always points to the top level of the build directory.
add_subdirectory(3rd_party)
add_definitions(-DACE_HAS_DLL=1) # TODO: this should be set by ACE library as a transitive definition
link_directories(${ThirdParty_Install_Dir}/lib)
include_directories(SYSTEM ${ThirdParty_Install_Dir}/include)


add_subdirectory(src)

if(ARAVIS_BUILD_VIEWER)
add_subdirectory(viewer)
endif()
add_subdirectory(qviewer)


