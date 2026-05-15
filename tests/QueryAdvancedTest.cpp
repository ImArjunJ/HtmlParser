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

    auto HighlightedTexts = Query.Select(".text.highlight");
    ASSERT_EQ(HighlightedTexts.size(), 1);
    ASSERT_EQ(HighlightedTexts[0]->GetTextContent(), "Paragraph 2");
}

TEST(QueryAdvancedTest, SelectByDescendantCombinator)
{
    std::string Html = R"(
    <div class="container">
        <section>
            <p>One</p>
            <p>Two</p>
        </section>
    </div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Paragraphs = Query.Select("div.container p");
    ASSERT_EQ(Paragraphs.size(), 2);
}

TEST(QueryAdvancedTest, SelectByChildCombinator)
{
    std::string Html = R"(
    <ul>
        <li>Direct</li>
        <li><span>Nested</span></li>
    </ul>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Items = Query.Select("ul > li");
    ASSERT_EQ(Items.size(), 2);

    auto NestedSpans = Query.Select("ul > span");
    ASSERT_TRUE(NestedSpans.empty());
}

TEST(QueryAdvancedTest, SelectByMultipleDescendantCombinators)
{
    std::string Html = R"(
    <div class="theirclass">
        <table>
            <tr>
                <td><a>One</a></td>
                <td><a>Two</a></td>
            </tr>
        </table>
    </div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Links = Query.Select("div.theirclass td a");
    ASSERT_EQ(Links.size(), 2);
}

TEST(QueryAdvancedTest, SelectBySiblingCombinators)
{
    std::string Html = R"(
    <div>
        <h2>Title</h2>
        <p>First</p>
        <p>Second</p>
    </div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    auto Adjacent = Query.Select("h2 + p");
    ASSERT_EQ(Adjacent.size(), 1);
    ASSERT_EQ(Adjacent[0]->GetTextContent(), "First");

    auto Following = Query.Select("h2 ~ p");
    ASSERT_EQ(Following.size(), 2);
}
