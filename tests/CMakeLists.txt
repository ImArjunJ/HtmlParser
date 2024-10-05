add_executable(RunTests ParserTest.cpp DomTest.cpp DomStrictTest.cpp QueryTest.cpp)
target_link_libraries(RunTests HtmlParser gtest gtest_main)

add_test(NAME ParserTest COMMAND RunTests)
