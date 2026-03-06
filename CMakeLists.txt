cmake_minimum_required(VERSION 3.20)
project(AHP-BASE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(yaml-cpp REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)   