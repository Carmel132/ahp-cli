cmake_minimum_required(VERSION 3.10)
project(AHP-BASE LANGUAGES CXX)

# Gather all .cpp files in src/
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include the header directory
target_include_directories(${PROJECT_NAME} PRIVATE include)   