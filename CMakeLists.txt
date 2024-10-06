cmake_minimum_required(VERSION 3.10)
project(HtmlParser)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(HtmlParser SHARED ${SOURCES})

if(MSVC)
    if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW) 
    endif()
    set_property(TARGET HtmlParser PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_target_properties(HtmlParser PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

option(HTMLPARSER_BUILD_TESTS "Build tests" ON)
option(HTMLPARSER_BUILD_BENCHMARKS "Build benchmarks" ON)
option(HTMLPARSER_BUILD_EXAMPLES "Build examples" ON)

if(HTMLPARSER_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(googletest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(tests)
endif()

if(HTMLPARSER_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(HTMLPARSER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
