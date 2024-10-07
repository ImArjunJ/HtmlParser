#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // Sample HTML input
    std::string Html = "<html><body><h1>Hello, World!</h1></body></html>";

    // Create a parser instance
    HtmlParser::Parser Parser;

    // Parse the HTML string
    HtmlParser::DOM DOM = Parser.Parse(Html);

    // Serialize the DOM back to HTML
    std::string SerializedHtml = DOM.ToHtml();

    // Output the serialized HTML
    std::cout << "Serialized HTML:\n" << SerializedHtml << "\n";

    return 0;
}
