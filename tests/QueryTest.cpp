#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>

TEST(QueryTest, SelectByTagName)
{
    std::string Html = R"(
    <div>
        <span>Text 1</span>
        <span>Text 2</span>
    </div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Spans = Query.Select("span");
    ASSERT_EQ(Spans.size(), 2);
}

TEST(QueryTest, SelectByClassName)
{
    std::string Html = R"(
    <p class="text">Paragraph 1</p>
    <p class="text highlight">Paragraph 2</p>
    <p>Paragraph 3</p>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Texts = Query.Select(".text");
    ASSERT_EQ(Texts.size(), 2);
}

TEST(QueryTest, SelectById)
{
    std::string Html = R"(
    <h1 id="title">Title</h1>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Title = Query.SelectFirst("#title");
    ASSERT_NE(Title, nullptr);
    ASSERT_EQ(Title->Tag, "h1");
}