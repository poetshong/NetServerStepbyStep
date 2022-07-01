cmake_minimum_required(VERSION 3.10) 

PROJECT(SERVER CXX)

# specify the C++ standard 
# set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_STANDARD 14) # for make_unique 
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

if(NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(Test)
endif()