#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(DOMTest, GetElementById)
{
    std::string Html = R"(
    <div id="main">Main Content</div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    auto Element = DOM.GetElementById("main");
    ASSERT_NE(Element, nullptr);
    ASSERT_EQ(Element->Tag, "div");
    ASSERT_EQ(Element->GetTextContent(), "Main Content");
}
TEST(DOMTest, GetElementsByClassName)
{
    std::string Html = R"(
    <div class="item">Item 1</div>
    <div class="item highlight">Item 2</div>
    <div class="highlight">Item 3</div>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    auto Items = DOM.GetElementsByClassName("item");
    ASSERT_EQ(Items.size(), 2);
}

TEST(DOMTest, GetElementsByTagName)
{
    std::string Html = R"(
    <ul>
        <li>First</li>
        <li>Second</li>
        <li>Third</li>
    </ul>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    auto ListItems = DOM.GetElementsByTagName("li");
    ASSERT_EQ(ListItems.size(), 3);
}