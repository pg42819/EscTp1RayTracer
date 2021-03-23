cmake_minimum_required(VERSION 3.17)
#cmake_minimum_required(VERSION 3.19)

# https://stackoverflow.com/questions/24380456/how-can-i-make-cmake-use-gcc-instead-of-clang-on-mac-os-x#
# Uncomment to force GCC (local to chris's system) instead of default clang
#set(CMAKE_C_COMPILER "/usr/local/bin/gcc-10")
#set(CMAKE_CXX_COMPILER "/usr/local/bin/gcc-10")
#set(CMAKE_ISPC_COMPILER "/usr/local/bin/ispc")

# see the compiler line verbose
set(CMAKE_VERBOSE_MAKEFILE ON)


project(RTracer CXX)
#project(RTracer C CXX ISPC)

if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # since 3.12
endif ()
if (POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW) # since 3.12
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_COMPILER_ID GNU)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
#include(cmake/gcc.cmake)
include(cmake/config.cmake)
include(cmake/ispc.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
endif ()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})

add_subdirectory(src)
