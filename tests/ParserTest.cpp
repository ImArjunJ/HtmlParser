#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(ParserTest, BasicParse)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");

    auto Body = DOM.GetElementsByTagName("body");
    ASSERT_EQ(Body.size(), 1);

    auto Paragraph = DOM.GetElementsByTagName("p");
    ASSERT_EQ(Paragraph.size(), 1);

    auto Text = Paragraph.front()->GetTextContent();
    ASSERT_EQ(Text, "Hello World");
}
