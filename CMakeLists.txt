cmake_minimum_required(VERSION 3.10)
project(HtmlParser)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(HtmlParser STATIC ${SOURCES})

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
    add_subdirectory(tests)
endif()

if(HTMLPARSER_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(HTMLPARSER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
