#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(DOMToHtmlTest, SerializesDOMToHtml)
{
    std::string Html = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <title>Test</title>
    </head>
    <body>
        <p>Sample text with & special < characters >.</p>
    </body>
    </html>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    std::string SerializedHtml = DOM.ToHtml();

    // Parse the serialized HTML again to compare
    HtmlParser::DOM DOM2 = Parser.Parse(SerializedHtml);

    // Compare the structures
    ASSERT_EQ(DOM.Root()->Children.size(), DOM2.Root()->Children.size());
}