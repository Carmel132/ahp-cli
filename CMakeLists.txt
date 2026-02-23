cmake_minimum_required(VERSION 3.20)
project(AHP-BASE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")   
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



find_package(yaml-cpp REQUIRED)

# Gather all .cpp files in src/
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

# Include the header directory
target_include_directories(${PROJECT_NAME} PRIVATE include)   