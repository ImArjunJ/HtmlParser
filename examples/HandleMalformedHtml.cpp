#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // Malformed HTML with missing closing tags
    std::string Html = "<div><p>Unclosed paragraph";

    HtmlParser::Parser Parser;

    // Non-strict mode (default behavior)
    try
    {
        HtmlParser::DOM DOM = Parser.Parse(Html);
        std::cout << "Parsed HTML in non-strict mode.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing HTML in non-strict mode: " << e.what() << "\n";
    }

    // Strict mode
    Parser.SetStrict(true);
    try
    {
        HtmlParser::DOM dom = Parser.Parse(Html);
        std::cout << "Parsed HTML in strict mode.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing HTML in strict mode: " << e.what() << "\n";
    }

    return 0;
}