#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML with an element that has an id attribute
    std::string Html = "<div id=\"main\">Main Content</div>";

    HtmlParser::Parser Parser;
    const HtmlParser::DOM DOM = Parser.Parse(Html);

    // Get the element with id "main"
    const auto Element = DOM.GetElementById("main");

    if (Element)
    {
        // Output the tag name and text content
        std::cout << "Found element: <" << Element->Tag << ">\n";
        std::cout << "Text content: " << Element->GetTextContent() << "\n";
    }
    else
    {
        std::cout << "Element with id 'main' not found.\n";
    }

    return 0;
}