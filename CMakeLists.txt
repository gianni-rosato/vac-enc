cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(vac C)

set (DESCRIPTION_SUMMARY
    "Fast and simple Opus command-line frontend")

message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release
        CACHE
        STRING "Build type: Debug, Release, RelWithDebInfo or MinSizeRel"
        FORCE)
endif()

if(WIN32)
    include_directories(${PROJECT_SOURCE_DIR}/win32)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(dep1 REQUIRED IMPORTED_TARGET libopusenc)
pkg_check_modules(dep2 REQUIRED IMPORTED_TARGET opus)
pkg_check_modules(dep3 REQUIRED IMPORTED_TARGET soxr)

add_executable(vac-enc
    src/decode.c
    src/flac.c
    src/main.c
    src/unicode_support.c
    src/wavreader.c)

target_link_libraries(vac-enc PUBLIC
        PkgConfig::dep1
        PkgConfig::dep2
        PkgConfig::dep3)

set_target_properties(vac-enc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")