#include <gtest/gtest.h>
#include <HtmlParser/Parser.hpp>
#include <HtmlParser/DOM.hpp>

TEST(ParserWhitespaceTest, WhitespaceBeforeHtml)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("    \n\t  <html><body></body></html>");
    auto Body = DOM.GetElementsByTagName("body");
    ASSERT_EQ(Body.size(), 1);
}

TEST(ParserWhitespaceTest, WhitespaceBeforeHead)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html>  \n\t <head></head><body></body></html>");
    auto Head = DOM.GetElementsByTagName("head");
    ASSERT_EQ(Head.size(), 1);
}

TEST(ParserWhitespaceTest, WhitespaceInHead)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><head>  \n\t </head><body></body></html>");
    auto Head = DOM.GetElementsByTagName("head");
    ASSERT_EQ(Head.size(), 1);
}

TEST(ParserWhitespaceTest, WhitespaceAfterHead)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><head></head>  \n\t <body></body></html>");
    auto Body = DOM.GetElementsByTagName("body");
    ASSERT_EQ(Body.size(), 1);
}

