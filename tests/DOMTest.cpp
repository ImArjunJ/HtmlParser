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

TEST(DOMTest, SetTextContentReplacesChildren)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<div><span>Old</span><em>Text</em></div>");

    auto Element = DOM.GetElementsByTagName("div").front();
    Element->SetTextContent("New text");

    ASSERT_EQ(Element->Children.size(), 1);
    ASSERT_EQ(Element->Children.front()->Type, HtmlParser::NodeType::Text);
    ASSERT_EQ(Element->GetTextContent(), "New text");
}

TEST(DOMTest, RemoveChildDetachesNode)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<ul><li id=\"remove\">Remove</li><li>Keep</li></ul>");

    auto Parent = DOM.GetElementsByTagName("ul").front();
    auto Child = DOM.GetElementById("remove");

    ASSERT_TRUE(Parent->RemoveChild(Child));
    ASSERT_EQ(Parent->Children.size(), 1);
    ASSERT_TRUE(Child->Parent.expired());
}

TEST(DOMTest, ReparentsAndReordersChildren)
{
    HtmlParser::Parser Parser;
    auto Document = Parser.Parse("<div id=a><i></i><b></b></div><div id=b></div>");
    auto First = Document.GetElementById("a");
    auto Second = Document.GetElementById("b");
    auto Italic = First->Children[0];
    auto Bold = First->Children[1];
    First->AppendChild(Italic);
    ASSERT_EQ(First->Children, (std::vector<std::shared_ptr<HtmlParser::Node>>{Bold, Italic}));
    First->InsertBefore(Italic, Bold);
    ASSERT_EQ(First->Children.front(), Italic);
    Second->AppendChild(Italic);
    ASSERT_EQ(First->Children.size(), 1);
    ASSERT_EQ(Italic->Parent.lock(), Second);
    Italic->Remove();
    ASSERT_TRUE(Second->Children.empty());
    ASSERT_TRUE(Italic->Parent.expired());
}

TEST(DOMTest, RejectsInvalidTreeMutations)
{
    auto Parent = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    auto Child = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    Parent->AppendChild(Child);
    EXPECT_THROW(Parent->AppendChild(nullptr), std::invalid_argument);
    EXPECT_THROW(Child->AppendChild(Parent), std::invalid_argument);
    EXPECT_THROW(Child->AppendChild(Child), std::invalid_argument);
    EXPECT_THROW(Child->InsertBefore(Parent, Parent), std::invalid_argument);
    EXPECT_EQ(Child->Parent.lock(), Parent);
}

TEST(DOMTest, AttributeNamesAreCaseInsensitive)
{
    HtmlParser::Node Element(HtmlParser::NodeType::Element);
    Element.SetAttribute("DATA-ID", "value");
    EXPECT_EQ(Element.GetAttribute("Data-Id"), "value");
    EXPECT_TRUE(Element.HasAttribute("data-id"));
    EXPECT_TRUE(Element.RemoveAttribute("DATA-id"));
    EXPECT_FALSE(Element.HasAttribute("data-id"));
}

TEST(DOMTest, MatchesAllRequestedClassesUsingHtmlWhitespace)
{
    HtmlParser::Parser Parser;
    auto Document = Parser.Parse("<i class='a b\tc\fd'></i><b class='a'></b>");
    EXPECT_EQ(Document.GetElementsByClassName("a c d").size(), 1);
    EXPECT_TRUE(Document.GetElementsByClassName(" ").empty());
    EXPECT_FALSE(Document.GetElementsByTagName("i")[0]->HasClass("a b"));
}

TEST(DOMTest, HandlesDeepTreesAndRetainedSubtrees)
{
    auto Root = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    Root->Tag = "div";
    auto Current = Root;
    for (int Index = 0; Index < 20000; ++Index)
    {
        auto Child = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
        Child->Tag = "div";
        Current->AppendChild(Child);
        Current = Child;
    }
    Current->SetTextContent("deep");
    EXPECT_EQ(Root->GetTextContent(), "deep");
    EXPECT_EQ(Root->ToHtml().size(), 20001 * 11 + 4);
    EXPECT_EQ(HtmlParser::DOM(Root).GetElementsByTagName("*").size(), 20001);
    auto Retained = Root->Children.front();
    Root.reset();
    EXPECT_TRUE(Retained->Parent.expired());
    EXPECT_EQ(Retained->GetTextContent(), "deep");
}

TEST(DOMTest, EmptyAndNullDocumentsAreUsable)
{
    EXPECT_TRUE(HtmlParser::DOM().ToHtml().empty());
    HtmlParser::DOM Document(nullptr);
    EXPECT_TRUE(Document.ToHtml().empty());
    EXPECT_TRUE(Document.GetElementsByTagName("*").empty());
    EXPECT_EQ(Document.GetElementById("missing"), nullptr);
}
