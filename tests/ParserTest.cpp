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

TEST(TokenizerTest, StreamsTokensAndResetsWithoutRetainingThem)
{
    HtmlParser::Tokenizer Tokenizer("<DIV a='x' A='y'>text</DIV>");
    auto Start = Tokenizer.NextToken();
    EXPECT_EQ(Start.Type, HtmlParser::TokenType::StartTag);
    EXPECT_EQ(Start.Data, "div");
    EXPECT_EQ(Start.Attributes.at("a"), "x");
    EXPECT_EQ(Start.Offset, 0);
    EXPECT_EQ(Tokenizer.NextToken().Data, "text");
    EXPECT_EQ(Tokenizer.NextToken().Type, HtmlParser::TokenType::EndTag);
    EXPECT_EQ(Tokenizer.NextToken().Type, HtmlParser::TokenType::EOFToken);
    EXPECT_TRUE(Tokenizer.GetTokens().empty());
    ASSERT_EQ(Tokenizer.GetErrors().size(), 1);
    Tokenizer.Tokenize();
    EXPECT_EQ(Tokenizer.GetTokens().size(), 3);
    EXPECT_EQ(Tokenizer.GetErrors().size(), 1);
}

TEST(TokenizerTest, PreservesRawTextAndDecodesRcdata)
{
    HtmlParser::Tokenizer Tokenizer("<script>if (x < y) s='&amp;<p>';</SCRIPT><textarea>&amp;<b>x</b></textarea>");
    EXPECT_EQ(Tokenizer.NextToken().Data, "script");
    EXPECT_EQ(Tokenizer.NextToken().Data, "if (x < y) s='&amp;<p>';");
    EXPECT_EQ(Tokenizer.NextToken().Data, "script");
    EXPECT_EQ(Tokenizer.NextToken().Data, "textarea");
    EXPECT_EQ(Tokenizer.NextToken().Data, "&<b>x</b>");
    EXPECT_EQ(Tokenizer.NextToken().Data, "textarea");
}

TEST(TokenizerTest, UsesAppropriateRawTextEndTags)
{
    HtmlParser::Tokenizer Tokenizer("<style>a</stylex>b</StYlE >after");
    Tokenizer.NextToken();
    EXPECT_EQ(Tokenizer.NextToken().Data, "a</stylex>b");
    EXPECT_EQ(Tokenizer.NextToken().Data, "style");
    EXPECT_EQ(Tokenizer.NextToken().Data, "after");
}

TEST(TokenizerTest, HandlesIncompleteMarkupAndLiteralLessThan)
{
    HtmlParser::Tokenizer Tokenizer("a < b < / c <");
    EXPECT_EQ(Tokenizer.NextToken().Data, "a < b < / c <");
    HtmlParser::Tokenizer Incomplete("<p a='unfinished");
    EXPECT_EQ(Incomplete.NextToken().Type, HtmlParser::TokenType::EOFToken);
    ASSERT_EQ(Incomplete.GetErrors().size(), 1);
    EXPECT_EQ(Incomplete.GetErrors()[0].Offset, 0);
}

TEST(TokenizerTest, DecodesFullHtmlEntitiesAndNumericReplacements)
{
    HtmlParser::Tokenizer Tokenizer("&copy; &NotEqualTilde; &CounterClockwiseContourIntegral; &#x1F600; &#128; &#0; &#xD800; &#999999999999999999999; &#65 &unknown;");
    EXPECT_EQ(Tokenizer.NextToken().Data, "\xC2\xA9 \xE2\x89\x82\xCC\xB8 \xE2\x88\xB3 \xF0\x9F\x98\x80 \xE2\x82\xAC \xEF\xBF\xBD \xEF\xBF\xBD \xEF\xBF\xBD A &unknown;");
}

TEST(TokenizerTest, DistinguishesAttributeEntityAmbiguity)
{
    HtmlParser::Tokenizer Tokenizer("<a x='&notit; &amp= &copy;'>&notit; &amp=</a>");
    EXPECT_EQ(Tokenizer.NextToken().Attributes.at("x"), "&notit; &amp= \xC2\xA9");
    EXPECT_EQ(Tokenizer.NextToken().Data, "\xC2\xACit; &=");
}

TEST(TokenizerTest, NormalizesNewlinesAndNullsAndReportsOriginalPositions)
{
    const std::string Input = std::string("a\r\nb\rc\0d", 8) + "\n<!--unfinished";
    HtmlParser::Tokenizer Tokenizer(Input);
    EXPECT_EQ(Tokenizer.NextToken().Data, "a\nb\nc\xEF\xBF\xBD" "d\n");
    EXPECT_EQ(Tokenizer.NextToken().Type, HtmlParser::TokenType::Comment);
    ASSERT_EQ(Tokenizer.GetErrors().size(), 1);
    EXPECT_EQ(Tokenizer.GetErrors()[0].Line, 4);
    EXPECT_EQ(Tokenizer.GetErrors()[0].Column, 1);
}
