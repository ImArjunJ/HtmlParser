#include <gtest/gtest.h>

#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Parser.hpp>

TEST(DOMToHtmlTest, SerializesDOMToHtml)
{
    std::string Html = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <title>Test</title>
    </head>
    <body>
        <p>Sample text with & special < characters >.</p>
    </body>
    </html>
    )";
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    std::string SerializedHtml = DOM.ToHtml();

    // Parse the serialized HTML again to compare
    HtmlParser::DOM DOM2 = Parser.Parse(SerializedHtml);

    // Compare the structures
    ASSERT_EQ(DOM.Root()->Children.size(), DOM2.Root()->Children.size());
}

TEST(DOMToHtmlTest, PreservesTextAfterVoidElement)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<p>before<img src=\"x\">after</p>");

    ASSERT_EQ(DOM.ToHtml(), "<html><head></head><body><p>before<img src=\"x\">after</p></body></html>");
}

TEST(DOMToHtmlTest, DoesNotDoubleEscapeDecodedEntities)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<p>A &amp; B</p>");

    ASSERT_EQ(DOM.ToHtml(), "<html><head></head><body><p>A &amp; B</p></body></html>");
}

TEST(DOMToHtmlTest, SerializesDoctypeAndComments)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<!DOCTYPE html><!--note--><html><body><p>x</p></body></html>");

    ASSERT_EQ(DOM.ToHtml(), "<!DOCTYPE html><!--note--><html><head></head><body><p>x</p></body></html>");
}

TEST(DOMToHtmlTest, SerializesAnElementWithDeterministicAttributes)
{
    auto Element = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    Element->Tag = "p";
    Element->SetAttribute("z", "&\"");
    Element->SetAttribute("a", "first");
    Element->SetTextContent("<text>");
    EXPECT_EQ(Element->ToHtml(), "<p a=\"first\" z=\"&amp;&quot;\">&lt;text&gt;</p>");
}

TEST(DOMToHtmlTest, PreservesRawTextWhenSerializing)
{
    auto Element = std::make_shared<HtmlParser::Node>(HtmlParser::NodeType::Element);
    Element->Tag = "script";
    Element->SetTextContent("if (a < b && c > d) run();");
    EXPECT_EQ(Element->ToHtml(), "<script>if (a < b && c > d) run();</script>");
}

TEST(DOMToHtmlTest, RoundTripsPreformattedLeadingNewlines)
{
    const HtmlParser::Parser Parser;
    auto Document = Parser.Parse("<pre>\n\ntext</pre><textarea>\n\nvalue</textarea><listing>\n\nlist</listing>");
    auto RoundTrip = Parser.Parse(Document.ToHtml());
    EXPECT_EQ(Document.Root()->GetTextContent(), RoundTrip.Root()->GetTextContent());
    EXPECT_EQ(Document.ToHtml(), RoundTrip.ToHtml());
}

TEST(DOMToHtmlTest, RoundTripsQuotedDoctypesAttributesAndRawText)
{
    const HtmlParser::Parser Parser;
    auto Document = Parser.Parse(
        "<!DOCTYPE html PUBLIC 'a>b'><html lang=en><head><title>A &lt; B</title><script>if (a < b && c) "
        "f();</script></head><body><!--note--><p x='&quot;&amp;&lt;'>&copy; &NotEqualTilde;</p></body></html>");
    auto RoundTrip = Parser.Parse(Document.ToHtml());
    EXPECT_EQ(Document.ToHtml(), RoundTrip.ToHtml());
    EXPECT_EQ(Document.Root()->GetTextContent(), RoundTrip.Root()->GetTextContent());
    EXPECT_EQ(RoundTrip.SelectFirst("p")->GetAttribute("x"), "\"&<");
}
