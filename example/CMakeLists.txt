cmake_minimum_required(VERSION 3.15)
project(OneOf)

set (CMAKE_CXX_STANDARD 17)

add_executable(one_of_example src/main.cpp)
target_include_directories(one_of_example PRIVATE ${CMAKE_SOURCE_DIR}/../include)

