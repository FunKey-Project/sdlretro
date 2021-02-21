cmake_minimum_required(VERSION 3.0)

if("${MODEL}" MATCHES "funkey")
    set(OPENDINGUX TRUE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/funkey-linux-gcc.cmake")
endif()

project(sdlretro)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(src)
