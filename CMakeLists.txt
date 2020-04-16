cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME satellite_lab1)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(SOURCES lib/argparse.hpp source/main.cpp source/ProReader.h source/ProReader.cpp  source/utils.h source/tests.h)

add_executable(slicer ${SOURCES})