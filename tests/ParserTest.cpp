#include <gtest/gtest.h>

#include <future>

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
    HtmlParser::Tokenizer Tokenizer("&copy; &NotEqualTilde; &CounterClockwiseContourIntegral; &#x1F600; &#128; &#0; "
                                    "&#xD800; &#999999999999999999999; &#65 &unknown;");
    EXPECT_EQ(Tokenizer.NextToken().Data, "\xC2\xA9 \xE2\x89\x82\xCC\xB8 \xE2\x88\xB3 \xF0\x9F\x98\x80 \xE2\x82\xAC "
                                          "\xEF\xBF\xBD \xEF\xBF\xBD \xEF\xBF\xBD A &unknown;");
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
    EXPECT_EQ(Tokenizer.NextToken().Data, "a\nb\nc\xEF\xBF\xBD"
                                          "d\n");
    EXPECT_EQ(Tokenizer.NextToken().Type, HtmlParser::TokenType::Comment);
    ASSERT_EQ(Tokenizer.GetErrors().size(), 1);
    EXPECT_EQ(Tokenizer.GetErrors()[0].Line, 4);
    EXPECT_EQ(Tokenizer.GetErrors()[0].Column, 1);
}

TEST(ParserTest, BuildsCompleteEmptyDocuments)
{
    EXPECT_EQ(HtmlParser::Parser().Parse("").ToHtml(), "<html><head></head><body></body></html>");
    EXPECT_EQ(HtmlParser::Parser().Parse("<!DOCTYPE html>").ToHtml(),
              "<!DOCTYPE html><html><head></head><body></body></html>");
}

TEST(ParserTest, PreservesHeadElementsAndBodyText)
{
    auto Document =
        HtmlParser::Parser().Parse("<html><head><meta charset=utf-8><title>A &amp; B</title><style>p > a { color: red; "
                                   "}</style></head><body>  hello <b>world</b> !</body></html>");
    auto Head = Document.GetElementsByTagName("head").front();
    ASSERT_EQ(Head->Children.size(), 3);
    EXPECT_EQ(Head->Children[1]->GetTextContent(), "A & B");
    EXPECT_EQ(Document.GetElementsByTagName("body").front()->GetTextContent(), "  hello world !");
    EXPECT_EQ(Document.GetElementsByTagName("style").front()->Children.size(), 1);
}

TEST(ParserTest, RecoversCommonOmittedEndTags)
{
    auto Document = HtmlParser::Parser().Parse(
        "<ul><li>one<li>two</ul><p>paragraph<div>block</div><select><option>a<option>b</select>");
    auto Items = Document.GetElementsByTagName("li");
    ASSERT_EQ(Items.size(), 2);
    EXPECT_EQ(Items[0]->GetTextContent(), "one");
    EXPECT_EQ(Items[0]->Parent.lock(), Items[1]->Parent.lock());
    EXPECT_EQ(Document.GetElementsByTagName("p")[0]->GetTextContent(), "paragraph");
    EXPECT_EQ(Document.GetElementsByTagName("option")[0]->GetTextContent(), "a");
}

TEST(ParserTest, BuildsImplicitTableContainers)
{
    auto Document = HtmlParser::Parser().Parse("<table><tr><td>one<td>two<tr><td>three</table>");
    auto Rows = Document.GetElementsByTagName("tr");
    ASSERT_EQ(Rows.size(), 2);
    EXPECT_EQ(Rows[0]->Parent.lock()->Tag, "tbody");
    EXPECT_EQ(Rows[0]->Children.size(), 2);
    EXPECT_EQ(Rows[1]->GetTextContent(), "three");
}

TEST(ParserTest, ParsesFragmentsAndContextText)
{
    const HtmlParser::Parser Parser;
    auto Fragment = Parser.ParseFragment("<p>a</p><p>b</p>");
    ASSERT_EQ(Fragment.Root()->Children.size(), 2);
    EXPECT_EQ(Fragment.ToHtml(), "<p>a</p><p>b</p>");
    EXPECT_EQ(Fragment.Root()->Children[0]->Parent.lock(), Fragment.Root());
    EXPECT_EQ(Parser.ParseFragment("<tr><td>x", "table").ToHtml(), "<tbody><tr><td>x</td></tr></tbody>");
    EXPECT_EQ(Parser.ParseFragment("<b>&amp;", "textarea").Root()->GetTextContent(), "<b>&");
    EXPECT_EQ(Parser.ParseFragment("if (a < b) x='&amp;';", "script").Root()->GetTextContent(),
              "if (a < b) x='&amp;';");
}

TEST(ParserTest, ReportsDiagnosticsAndRecoversAcrossCalls)
{
    HtmlParser::Parser Parser;
    auto Result = Parser.ParseWithDiagnostics("<div>\r\n</bad></div>");
    ASSERT_EQ(Result.Errors.size(), 1);
    EXPECT_EQ(Result.Errors[0].Line, 2);
    EXPECT_EQ(Result.Errors[0].Column, 1);
    EXPECT_EQ(Result.Errors[0].Offset, 7);
    Parser.SetStrict(true);
    EXPECT_THROW(Parser.Parse("<p a='unfinished"), HtmlParser::ParseException);
    EXPECT_EQ(Parser.Parse("<p>ok</p>").GetElementsByTagName("p")[0]->GetTextContent(), "ok");
    EXPECT_EQ(Result.Document.GetElementsByTagName("div").size(), 1);
}

TEST(ParserTest, EnforcesResourceLimitsInRecoveryMode)
{
    HtmlParser::ParseOptions Options;
    Options.MaxDepth = 3;
    EXPECT_THROW(HtmlParser::Parser(Options).Parse("<div><b>x</b></div>"), HtmlParser::ParseException);
    EXPECT_NO_THROW(HtmlParser::Parser(Options).Parse("<div>x</div>"));
    Options.MaxDepth = 0;
    Options.MaxNodes = 4;
    EXPECT_THROW(HtmlParser::Parser(Options).Parse("<div>x</div>"), HtmlParser::ParseException);
    Options.MaxNodes = 0;
    Options.MaxErrors = 2;
    auto Result = HtmlParser::Parser(Options).ParseWithDiagnostics("</x></y></z>");
    EXPECT_EQ(Result.Errors.size(), 2);
}

TEST(ParserTest, IgnoresSelfClosingSyntaxOnHtmlNonVoidElements)
{
    auto Document = HtmlParser::Parser().Parse("<div/>text<br/>after</div>");
    EXPECT_EQ(Document.GetElementsByTagName("div")[0]->GetTextContent(), "textafter");
    EXPECT_EQ(Document.GetElementsByTagName("br")[0]->Children.size(), 0);
}

TEST(ParserTest, StripsOnlyTheImmediateLeadingPreformattedNewline)
{
    const HtmlParser::Parser Parser;
    auto Document = Parser.Parse("<pre>\r\n\ntext</pre><textarea>\nvalue</textarea><pre><!--x-->\nkeep</pre>");
    auto Pre = Document.GetElementsByTagName("pre");
    EXPECT_EQ(Pre[0]->GetTextContent(), "\ntext");
    EXPECT_EQ(Pre[1]->GetTextContent(), "\nkeep");
    EXPECT_EQ(Document.GetElementsByTagName("textarea")[0]->GetTextContent(), "value");
}

TEST(ParserTest, RecoversMisplacedDocumentClosersInHeadContent)
{
    auto Document = HtmlParser::Parser().Parse("<head><template></body></html></head><p>after");
    ASSERT_EQ(Document.GetElementsByTagName("body").size(), 1);
    EXPECT_EQ(Document.GetElementsByTagName("p").front()->GetTextContent(), "after");
    EXPECT_EQ(Document.GetElementsByTagName("p").front()->Parent.lock()->Tag, "body");
}

TEST(ParserTest, ReusesOneParserAcrossConcurrentCalls)
{
    const HtmlParser::Parser Parser;
    std::vector<std::future<bool>> Tasks;
    for (int Index = 0; Index < 8; ++Index)
        Tasks.push_back(std::async(
            std::launch::async,
            [&Parser, Index]
            {
                const auto Text = std::to_string(Index);
                for (int Iteration = 0; Iteration < 50; ++Iteration)
                    if (Parser.ParseFragment("<p>" + Text + "</p>").SelectFirst("p")->GetTextContent() != Text)
                        return false;
                return true;
            }));
    for (auto& Task : Tasks)
        EXPECT_TRUE(Task.get());
}

TEST(TokenizerTest, PreservesLargeTextRunsAndUnknownReferences)
{
    const std::string Prefix(100000, 'x');
    HtmlParser::Tokenizer Tokens(Prefix + " < 3 << /tail &amp; end <b>");
    EXPECT_EQ(Tokens.NextToken().Data, Prefix + " < 3 << /tail & end ");
    EXPECT_EQ(Tokens.NextToken().Data, "b");
    const std::string Unknown = "&" + std::string(100000, 'x') + ";";
    EXPECT_EQ(HtmlParser::Tokenizer(Unknown).NextToken().Data, Unknown);
}
