project(LmdFit)

cmake_minimum_required(VERSION 3.4)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Setting policy CMP0060 to the new version enforces that
# libraries are linked using their full path. That should help in
# case that multiple versions of a library (e.g. boost) are
# installed on a system
cmake_policy(SET CMP0060 NEW)

# Which target should be build by default?
set(DEFAULT_BUILD_TYPE "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
    STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  message(STATUS "CMAKE_CXX_STANDARD is not defined. Setting to 17")
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Compiler: " ${CMAKE_CXX_COMPILER_ID} " ( " ${CMAKE_CXX_COMPILER} " )")
message(STATUS "All possible build types and flags are listed below:")
message(STATUS "DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "RELWITHDEBINFO is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "MINSIZEREL is ${CMAKE_CXX_FLAGS_MINSIZEREL}")
message(STATUS "Use this CMake flag to select build type: -DCMAKE_BUILD_TYPE=BUILD_TYPE")
message(STATUS "Using build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable -fPIC flag
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Boost COMPONENTS
  thread
  system
  regex
  filesystem
  serialization
  REQUIRED)
find_package(ROOT REQUIRED)
find_package(PANDAROOT)
find_package(KOALASOFT)

include_directories(./)
include_directories(./model_framework)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(model_framework)

add_subdirectory(data)
add_subdirectory(fit)
add_subdirectory(model)
add_subdirectory(ui)

add_subdirectory(apps)
