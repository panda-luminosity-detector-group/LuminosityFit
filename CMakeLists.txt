cmake_minimum_required(VERSION 2.6)

project(LmdFit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")

include_directories(./)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(data)
add_subdirectory(fit)
add_subdirectory(model)
add_subdirectory(ui)

add_subdirectory(apps)
