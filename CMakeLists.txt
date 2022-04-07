cmake_minimum_required(VERSION 3.21)
project(comtest)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(/usr/include)
add_executable(comtest
        comtest.cpp
        comtest.h MessageHandler.cpp MessageHandler.h MessagePrinter.cpp MessagePrinter.h "common .h" AutoMeasure.cpp AutoMeasure.h MessageHandlerEnternet.cpp MessageHandlerEnternet.h RS232Comm.cpp RS232Comm.h)
