file(GLOB BENCHMARKS "*.cpp")

foreach(BENCHMARK_FILE ${BENCHMARKS})
    get_filename_component(BENCHMARK_NAME ${BENCHMARK_FILE} NAME_WE)
    add_executable(${BENCHMARK_NAME} ${BENCHMARK_FILE})
    target_link_libraries(${BENCHMARK_NAME} HtmlParser)
endforeach()