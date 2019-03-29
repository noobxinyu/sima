cmake_minimum_required(VERSION 3.13)
project(sima)

set(CMAKE_CXX_STANDARD 14)

add_executable(sima main.cpp)

target_link_libraries(sima
        PRIVATE
        boost_filesystem
        boost_system
        pthread
        )