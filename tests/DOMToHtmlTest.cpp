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

TEST(DOMToHtmlTest, PreservesTextAfterVoidElement)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<p>before<img src=\"x\">after</p>");

    ASSERT_EQ(DOM.ToHtml(), "<html><head></head><body><p>before<img src=\"x\">after</p></body></html>");
}

TEST(DOMToHtmlTest, DoesNotDoubleEscapeDecodedEntities)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<p>A &amp; B</p>");

    ASSERT_EQ(DOM.ToHtml(), "<html><head></head><body><p>A &amp; B</p></body></html>");
}

TEST(DOMToHtmlTest, SerializesDoctypeAndComments)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<!DOCTYPE html><!--note--><html><body><p>x</p></body></html>");

    ASSERT_EQ(DOM.ToHtml(), "<!DOCTYPE html><!--note--><html><head></head><body><p>x</p></body></html>");
}
