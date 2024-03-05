cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(vac C)

message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O3 -flto")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(PkgConfig REQUIRED)

pkg_check_modules(dep1 REQUIRED IMPORTED_TARGET OPUS)
pkg_check_modules(dep2 REQUIRED IMPORTED_TARGET LIBOPUSENC)
pkg_check_modules(dep3 REQUIRED IMPORTED_TARGET SOXR)

add_executable(vac-enc src/main.c src/wavreader.c)

target_link_libraries(vac-enc PUBLIC
        PkgConfig::dep1
        PkgConfig::dep2
    	PkgConfig::dep3)
