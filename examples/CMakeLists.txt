set(EXAMPLES
    ParseExample.cpp
    QueryExample.cpp
    RootsExample.cpp
)

foreach(EXAMPLE_FILE ${EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    target_link_libraries(${EXAMPLE_NAME} HtmlParser)
endforeach()