#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML with nested elements
    std::string Html = R"(
        <div>
            <h1>Welcome</h1>
            <p>This is a <strong>sample</strong> paragraph with <em>nested</em> elements.</p>
        </div>
    )";

    HtmlParser::Parser Parser;
    const HtmlParser::DOM DOM = Parser.Parse(Html);

    // Get the <p> element
    const auto Paragraph = DOM.GetElementsByTagName("p").front();

    if (Paragraph)
    {
        // Extract the text content, including text from nested elements
        const std::string Text = Paragraph->GetTextContent();
        std::cout << "Paragraph text: " << Text << "\n";
    }
    else
    {
        std::cout << "Paragraph not found.\n";
    }

    return 0;
}