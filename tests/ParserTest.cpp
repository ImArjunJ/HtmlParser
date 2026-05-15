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

TEST(ParserTest, CoalescesTextRuns)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<a>Some text</a>");

    auto Anchor = DOM.GetElementsByTagName("a").front();
    ASSERT_EQ(Anchor->Children.size(), 1);
    ASSERT_EQ(Anchor->Children.front()->Type, HtmlParser::NodeType::Text);
    ASSERT_EQ(Anchor->Children.front()->Text, "Some text");
}

TEST(ParserTest, DecodesTextEntities)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<p>A &amp; B &lt; C</p>");

    auto Paragraph = DOM.GetElementsByTagName("p").front();
    ASSERT_EQ(Paragraph->GetTextContent(), "A & B < C");
}
