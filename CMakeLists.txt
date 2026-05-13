cmake_minimum_required(VERSION 3.20)
project(factory-simulation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(factory-simulation
        src/main.cpp
        src/simulator.cpp
        src/main.cpp
        src/simulator.cpp
        include/simulator.h
)