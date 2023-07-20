# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)



#idf_component_register(SRCS "${CMAKE_SOURCE_DIR}/main/main.cpp")

#add_subdirectory(${CMAKE_SOURCE_DIR}/main)
#include(${CMAKE_SOURCE_DIR}/main/CMakeLists.txt)
#add_subdirectory(${CMAKE_SOURCE_DIR}/../common)

#set(COMMON_BINARY_DIRs "${CMAKE_BINARY_DIR}/common")
#add_subdirectory(${CMAKE_SOURCE_DIR}/../common $COMMON_BINARY_DIR)

project(open_stream_sign)
#include(${CMAKE_SOURCE_DIR}/../common/CMakeLists.txt)
