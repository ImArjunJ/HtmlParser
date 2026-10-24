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

TEST(QueryAdvancedTest, SelectorListsAreUniqueAndInDocumentOrder)
{
    auto Document = HtmlParser::Parser().Parse("<div id=a><p id=one></p><div id=b><p id=two></p></div><p id=three></p></div>");
    auto Matches = Document.Select("#three, div p, #one");
    ASSERT_EQ(Matches.size(), 3);
    EXPECT_EQ(Matches[0]->GetAttribute("id"), "one");
    EXPECT_EQ(Matches[1]->GetAttribute("id"), "two");
    EXPECT_EQ(Matches[2]->GetAttribute("id"), "three");
    EXPECT_EQ(Document.SelectFirst("#three, #one"), Matches[0]);
}

TEST(QueryAdvancedTest, HandlesQuotedAttributeSyntaxAndMissingAttributes)
{
    auto Document = HtmlParser::Parser().Parse("<a title='x > y, [z]' empty='' lang=en-US data-value='Hello world.txt'></a><a></a>");
    EXPECT_EQ(Document.Select("[title='x > y, [z]']").size(), 1);
    EXPECT_EQ(Document.Select("[empty='']").size(), 1);
    EXPECT_TRUE(Document.Select("[missing='']").empty());
    EXPECT_EQ(Document.Select("[lang|=en]").size(), 1);
    EXPECT_EQ(Document.Select("[data-value^=hello i][data-value$='.txt'][data-value*='world'][data-value~='Hello']").size(), 1);
    EXPECT_TRUE(Document.Select("[data-value^='']").empty());
}

TEST(QueryAdvancedTest, StructuralPseudoClassesIgnoreNonElementSiblings)
{
    auto Document = HtmlParser::Parser().Parse("<ul>text<!--x--><li id=a></li><b></b><li id=b></li><li id=c></li></ul>");
    EXPECT_EQ(Document.SelectFirst("li:first-child")->GetAttribute("id"), "a");
    EXPECT_EQ(Document.SelectFirst("li:last-child")->GetAttribute("id"), "c");
    EXPECT_EQ(Document.SelectFirst("li:nth-child(3)")->GetAttribute("id"), "b");
    EXPECT_EQ(Document.SelectFirst("li:nth-of-type(2)")->GetAttribute("id"), "b");
    EXPECT_EQ(Document.Select("li:nth-of-type(2n + 1)").size(), 2);
    EXPECT_EQ(Document.Select("li:nth-last-of-type(-n+2)").size(), 2);
    EXPECT_EQ(Document.SelectFirst("li:nth-last-child(1)")->GetAttribute("id"), "c");
    EXPECT_EQ(Document.Select("b:only-of-type").size(), 1);
    EXPECT_EQ(Document.Select("li:only-child").size(), 0);
    EXPECT_EQ(Document.Select("li:empty").size(), 3);
    EXPECT_EQ(Document.Select("ul:empty").size(), 0);
}

TEST(QueryAdvancedTest, SupportsNestedLogicalSelectorsAndScope)
{
    auto Document = HtmlParser::Parser().Parse("<div id=root><p class=a></p><section><p class=b></p></section><p class=c></p></div>");
    auto Root = Document.GetElementById("root");
    EXPECT_EQ(Root->Select(":scope > p:not(.a, .b)").size(), 1);
    EXPECT_EQ(Root->Select("p:is(.a, .b):not(:where(.c))").size(), 2);
    EXPECT_EQ(Document.Select(":root").size(), 1);
    EXPECT_EQ(Root->SelectFirst(":scope"), Root);
    EXPECT_EQ(Root->Select("body p").size(), 0);
    auto Nested = Root->SelectFirst("section p");
    ASSERT_NE(Nested, nullptr);
    EXPECT_TRUE(Nested->Matches("section > p.b"));
    EXPECT_EQ(Nested->Closest("div"), Root);
    EXPECT_EQ(Nested->Closest(":scope"), Nested);
}

TEST(QueryAdvancedTest, SupportsEscapedIdentifiersAndReusableSelectors)
{
    auto Document = HtmlParser::Parser().Parse("<p id='123' class='a:b'></p><p id=other></p>");
    HtmlParser::Selector Pattern(R"(#\31 23.a\:b)");
    ASSERT_TRUE(Pattern.IsValid()) << Pattern.Error();
    auto Copy = Pattern;
    EXPECT_EQ(HtmlParser::Query(Document.Root()).Select(Copy).size(), 1);
    EXPECT_TRUE(Pattern.Matches(*Document.GetElementById("123")));
    auto Moved = std::move(Copy);
    EXPECT_TRUE(Moved.IsValid());
    EXPECT_FALSE(Copy.IsValid());
    EXPECT_EQ(HtmlParser::Query(Document.Root()).SelectFirst(Moved), Document.GetElementById("123"));
}

TEST(QueryAdvancedTest, RejectsMalformedAndUnsupportedSelectors)
{
    auto Document = HtmlParser::Parser().Parse("<p id=x></p>");
    for (const auto* Input : {"", ".", "#", "p,", "p >", "> p", "p + > p", "[x", "[x='oops]", ":unknown", ":not()", ":nth-child(foo)", "p::before", "[x!=y]"})
    {
        HtmlParser::Selector Pattern(Input);
        EXPECT_FALSE(Pattern.IsValid()) << Input;
        EXPECT_FALSE(Pattern.Error().empty()) << Input;
        EXPECT_TRUE(Document.Select(Input).empty()) << Input;
    }
    EXPECT_TRUE(HtmlParser::Query(nullptr).Select("*").empty());
    EXPECT_EQ(HtmlParser::Query(nullptr).SelectFirst("*"), nullptr);
}

TEST(QueryAdvancedTest, BacktracksAcrossAncestorAndSiblingCombinators)
{
    auto Document = HtmlParser::Parser().Parse("<div class=a><div><div class=a><p id=x></p></div></div></div><i></i><i></i><p id=y></p>");
    EXPECT_EQ(Document.Select("div.a > div p").size(), 1);
    EXPECT_EQ(Document.Select("i ~ i + p").size(), 1);
    EXPECT_EQ(Document.Select("i + p")[0]->GetAttribute("id"), "y");
}

TEST(QueryAdvancedTest, StructuralSelectorsWorkOnDetachedRoots)
{
    auto Root = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    Root->Tag = "div";
    auto Child = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    Child->Tag = "p";
    Root->AppendChild(Child);
    auto Matches = Root->Select(":first-child");
    ASSERT_EQ(Matches.size(), 2);
    EXPECT_EQ(Matches[0], Root);
    EXPECT_EQ(Matches[1], Child);
}

TEST(QueryAdvancedTest, HandlesAmbiguousDeepSelectorChains)
{
    std::string Html;
    for (int Index = 0; Index < 100; ++Index)
        Html += "<div>";
    Html += "<p>x</p>";
    auto Document = HtmlParser::Parser().Parse(Html);
    EXPECT_TRUE(Document.Select(".missing div div div div div div div div p").empty());
}
