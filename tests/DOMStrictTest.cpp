#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(DOMTest, SerializeToHTMLStrict)
{
    HtmlParser::Parser Parser;
    Parser.SetStrict(true);
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");
    ASSERT_EQ(DOM.ToHtml(), "<html><body><p>Hello World</p></body></html>");

    // Test strict mode
    ASSERT_THROW(Parser.Parse("<html><body><p>Hello World</body></html>"), std::runtime_error);
}