cmake_minimum_required(VERSION 3.20)
project(satpos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html
# set(CMAKE_PREFIX_PATH $ENV{CXX_INSTALL_DIR})

include(cmake/Protobuf.cmake)
include(cmake/GRPC.cmake)

add_subdirectory(third_party)
add_subdirectory(src)
