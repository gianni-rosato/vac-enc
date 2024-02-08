cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

project(vac C)

set (DESCRIPTION_SUMMARY
    "Simple, high quality CLI opus frontend")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release
        CACHE
        STRING "Build type: Debug, Release, RelWithDebInfo or MinSizeRel"
        FORCE)
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(dep1 REQUIRED IMPORTED_TARGET OPUS) 
pkg_check_modules(dep2 REQUIRED IMPORTED_TARGET LIBOPUSENC)
pkg_check_modules(dep3 REQUIRED IMPORTED_TARGET SOXR)

include_directories (${PROJECT_SOURCE_DIR}/src)

add_subdirectory (src/)

set_target_properties(vac-enc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
