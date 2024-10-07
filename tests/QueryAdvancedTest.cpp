#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>

TEST(QueryAdvancedTest, SelectByAttribute)
{
    std::string Html = R"(
    <input type="text" name="username">
    <input type="password" name="password">
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto TextInputs = Query.Select("input[type=text]");
    ASSERT_EQ(TextInputs.size(), 1);
    ASSERT_EQ(TextInputs[0]->GetAttribute("name"), "username");
}

TEST(QueryAdvancedTest, SelectByCombinedSelectors)
{
    std::string Html = R"(
    <div class="container">
        <p class="text">Paragraph 1</p>
        <p class="text highlight">Paragraph 2</p>
    </div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto HighlightedTexts = Query.Select("div.container .text.highlight");
    ASSERT_EQ(HighlightedTexts.size(), 1);
    ASSERT_EQ(HighlightedTexts[0]->GetTextContent(), "Paragraph 2");
}