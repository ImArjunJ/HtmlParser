foreach(NAME IN ITEMS CompiledSelectors ExtractTextContent GetElementById GetElementsByClassName
    HandleMalformedHtml ModifyAttributes ParseAndSerialize ParseHtmlFragment QuerySelectors RemoveElement
    StreamingTokens TraverseDOM)
    add_executable(${NAME} ${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE HtmlParser::HtmlParser)
endforeach()
