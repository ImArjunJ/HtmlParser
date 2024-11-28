#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(DOMStrictTest, ThrowsOnMalformedHTML)
{
    std::string Html = R"(
    <div>
        <p>Unclosed paragraph
    </div>
    )";
    HtmlParser::Parser Parser;
    Parser.SetStrict(true);

    ASSERT_THROW({ HtmlParser::DOM DOM = Parser.Parse(Html); }, std::runtime_error);
}

TEST(DOMStrictTest, ParsesWellFormedHTMLInStrictMode)
{
    std::string Html = R"(
    <div>
        <p>Closed paragraph</p>
    </div>
    )";
    HtmlParser::Parser Parser;
    Parser.SetStrict(true);

    ASSERT_NO_THROW({
        HtmlParser::DOM DOM = Parser.Parse(Html);
        auto Root = DOM.Root();
        ASSERT_EQ(Root->Children.size(), 1);
        auto HtmlElement = Root->Children[0];
        ASSERT_EQ(HtmlElement->Tag, "html");
    });
}