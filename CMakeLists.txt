cmake_minimum_required(VERSION 3.25)
project(connectfour)

set(CMAKE_CXX_STANDARD 20)

add_executable(connectfour main.cpp Judge.cpp Strategy.cpp MyEngine.cpp)
