#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(DOMTest, SerializeToHTML)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");
    ASSERT_EQ(DOM.ToHtml(), "<html><body><p>Hello World</p></body></html>");
}
