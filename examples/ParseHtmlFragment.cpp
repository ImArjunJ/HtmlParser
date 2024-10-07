#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML fragment without a single root element
    std::string Html = "<p>Paragraph 1</p><p>Paragraph 2</p><p>Paragraph 3</p>";

    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    // Get all <p> elements
    auto Paragraphs = DOM.GetElementsByTagName("p");

    std::cout << "Found " << Paragraphs.size() << " paragraphs:\n";

    for (const auto& Paragraph : Paragraphs)
    {
        std::cout << "- " << Paragraph->GetTextContent() << "\n";
    }

    return 0;
}