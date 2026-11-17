cmake_minimum_required(VERSION 3.20)
project(labs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lab1 labs/1/main.cpp)
add_executable(lab2 labs/2/main.cpp)
add_executable(lab3 labs/3/main.cpp)
add_executable(lab4 labs/4/main.cpp)
add_executable(lab5 labs/5/main.cpp)
add_executable(lab5_1 labs/5.1/main.cpp)

