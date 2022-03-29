cmake_minimum_required(VERSION 3.21)
project(comtest)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(comtest
        comtest.cpp
        comtest.h MessageHandler.cpp MessageHandler.h MessagePrinter.cpp MessagePrinter.h "common .h")
