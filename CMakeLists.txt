cmake_minimum_required(VERSION 3.21)
project(comtest)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
# see https://cmake.org/cmake/help/latest/module/FindBoost.html

find_package(Boost REQUIRED iostreams system filesystem)
include_directories(${Boost_INCLUDE_DIR})
add_executable(comtest
        comtest.cpp
        comtest.h MessageHandler.cpp MessageHandler.h MessagePrinter.cpp MessagePrinter.h "common .h" AutoMeasure.cpp AutoMeasure.h MessageHandlerEnternet.cpp MessageHandlerEnternet.h RS232Comm.cpp RS232Comm.h)
target_link_libraries(comtest Boost::iostreams Boost::system Boost::filesystem)