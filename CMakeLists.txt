cmake_minimum_required(VERSION 3.10.2)
project(mqtt-ws-client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
# find_package(GTest REQUIRED)

link_directories(
    lib/mongoose
)

message(STATUS "BUILD BEGINING")

# adding execution nodes
add_executable(main src/main.cpp lib/mongoose/mongoose.c)


message(STATUS "BUILD FINISHED")