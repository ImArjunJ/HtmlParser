cmake_minimum_required(VERSION 3.16)
project(HtmlParser VERSION 0.2.0 LANGUAGES CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(HTMLPARSER_BUILD_OPTION_DEFAULT ON)
else()
    set(HTMLPARSER_BUILD_OPTION_DEFAULT OFF)
endif()

option(HTMLPARSER_BUILD_TESTS "Build tests" ${HTMLPARSER_BUILD_OPTION_DEFAULT})
option(HTMLPARSER_BUILD_EXAMPLES "Build examples" ${HTMLPARSER_BUILD_OPTION_DEFAULT})
option(HTMLPARSER_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

include(GNUInstallDirs)
add_library(HtmlParser
    src/DOM.cpp
    src/Node.cpp
    src/Parser.cpp
    src/Query.cpp
    src/Tokenizer.cpp
    src/Text.cpp
)
add_library(HtmlParser::HtmlParser ALIAS HtmlParser)
target_compile_features(HtmlParser PUBLIC cxx_std_20)
target_include_directories(HtmlParser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(HtmlParser PROPERTIES
    CXX_EXTENSIONS OFF
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
if(MSVC)
    target_compile_options(HtmlParser PRIVATE /W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(HtmlParser PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(HTMLPARSER_ENABLE_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" OR MSVC)
        message(FATAL_ERROR "Sanitizers require GCC or Clang with the GNU command-line interface")
    endif()
    target_compile_options(HtmlParser PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(HtmlParser PUBLIC -fsanitize=address,undefined)
endif()

if(HTMLPARSER_BUILD_TESTS)
    enable_testing()
    find_package(GTest 1.12 QUIET)
    if(NOT TARGET GTest::gtest_main)
        include(FetchContent)
        FetchContent_Declare(googletest
            URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
            URL_HASH SHA256=24564e3b712d3eb30ac9a85d92f7d720f60cc0173730ac166f27dda7fed76cb2
        )
        set(gtest_force_shared_crt ON)
        set(INSTALL_GTEST OFF)
        set(BUILD_GMOCK OFF)
        FetchContent_MakeAvailable(googletest)
    endif()
    add_subdirectory(tests)
endif()
if(HTMLPARSER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/HtmlParserConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HtmlParserConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HtmlParser
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/HtmlParserConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMinorVersion
)
install(TARGETS HtmlParser EXPORT HtmlParserTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT HtmlParserTargets NAMESPACE HtmlParser:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HtmlParser)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/HtmlParserConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HtmlParserConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HtmlParser
)
