cmake_minimum_required(VERSION 3.10) 

PROJECT(ECHOSERVER CXX)

# specify the C++ standard 
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_STANDARD_REQUIRED True) 

add_subdirectory(src bin)