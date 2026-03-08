cmake_minimum_required(VERSION 3.16)

project(cac)
set(CMAKE_CXX_COMPILER_ID "GNU")
set (CMAKE_CXX_STANDARD 26)
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE MinSizeRel)

add_executable(${PROJECT_NAME}
    src/interpreter.cpp
    src/lexer.cpp
    src/parser.cpp
    src/expressions.cpp
)
