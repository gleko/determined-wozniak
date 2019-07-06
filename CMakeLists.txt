cmake_minimum_required (VERSION 3.0)
project(DeterminedWozniak)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(DeterminedWozniak src/gui.cpp)

target_include_directories(DeterminedWozniak PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/incl>
    $<INSTALL_INTERFACE:incl>)

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-std=c++11")
enable_cxx_compiler_flag_if_supported("-O3")

find_package(SDL2 REQUIRED)

