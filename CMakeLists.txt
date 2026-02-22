cmake_minimum_required(VERSION 3.20)
project(AHP-BASE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(yaml-cpp REQUIRED)

# Gather all .cpp files in src/
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

# Include the header directory
target_include_directories(${PROJECT_NAME} PRIVATE include)   