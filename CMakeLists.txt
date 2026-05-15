cmake_minimum_required(VERSION 3.10)
project(HtmlParser)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(HTMLPARSER_BUILD_OPTION_DEFAULT ON)
else()
    set(HTMLPARSER_BUILD_OPTION_DEFAULT OFF)
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(HtmlParser STATIC ${SOURCES})
add_library(HtmlParser::HtmlParser ALIAS HtmlParser)
target_compile_features(HtmlParser PUBLIC cxx_std_20)
target_include_directories(HtmlParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(HTMLPARSER_BUILD_TESTS "Build tests" ${HTMLPARSER_BUILD_OPTION_DEFAULT})
option(HTMLPARSER_BUILD_BENCHMARKS "Build benchmarks" ${HTMLPARSER_BUILD_OPTION_DEFAULT})
option(HTMLPARSER_BUILD_EXAMPLES "Build examples" ${HTMLPARSER_BUILD_OPTION_DEFAULT})

if(HTMLPARSER_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()

if(HTMLPARSER_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(HTMLPARSER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
