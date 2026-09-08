cmake_minimum_required(VERSION 3.28.0 FATAL_ERROR)

# set build type to Release if nothing was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" FORCE)
endif()

project(iopp)

# set C++ build flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")

# create interface library
add_library(iopp INTERFACE)
target_include_directories(iopp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# provide tests and benchmark if standalone
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    enable_testing()
    add_subdirectory(test)
endif()
