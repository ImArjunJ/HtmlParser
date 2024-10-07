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

        // Filter out text nodes
        std::vector<std::shared_ptr<HtmlParser::Node>> ElementChildren;
        for (const auto& Child : HtmlElement->Children)
        {
            if (Child->NodeType == HtmlParser::NodeType::Element)
            {
                ElementChildren.push_back(Child);
            }
        }
        ASSERT_EQ(ElementChildren.size(), 1);

        auto BodyElement = ElementChildren[0];
        ASSERT_EQ(BodyElement->Tag, "body");
    });
}