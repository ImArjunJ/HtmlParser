add_executable(RunTests
    ParserTest.cpp
    DOMTest.cpp
    DOMStrictTest.cpp
    QueryTest.cpp
    DOMToHtmlTest.cpp
    QueryAdvancedTest.cpp
    WhitespaceTest.cpp
)
target_link_libraries(RunTests PRIVATE HtmlParser::HtmlParser GTest::gtest_main)
if(HTMLPARSER_ENABLE_SANITIZERS)
    target_compile_options(RunTests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()
include(GoogleTest)
gtest_discover_tests(RunTests)

foreach(MODE IN ITEMS installed subdirectory)
    add_test(NAME CMakeConsumer.${MODE}
        COMMAND ${CMAKE_COMMAND}
            -DHTMLPARSER_SOURCE_DIR=${PROJECT_SOURCE_DIR}
            -DHTMLPARSER_BINARY_DIR=${PROJECT_BINARY_DIR}
            -DHTMLPARSER_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DHTMLPARSER_GENERATOR=${CMAKE_GENERATOR}
            -DHTMLPARSER_CONFIG=$<CONFIG>
            -DHTMLPARSER_CONSUMER_MODE=${MODE}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunConsumerTest.cmake
    )
endforeach()
