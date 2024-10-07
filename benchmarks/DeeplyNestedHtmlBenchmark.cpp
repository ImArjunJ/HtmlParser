#include <HtmlParser/Parser.hpp>
#include <chrono>
#include <iostream>
#include <sstream>

int main()
{
    // Generate deeply nested HTML
    const std::size_t NestingLevel = 1000;
    std::ostringstream HtmlStream;

    for (std::size_t i = 0; i < NestingLevel; ++i)
    {
        HtmlStream << "<div>";
    }
    HtmlStream << "Nested Content";
    for (std::size_t i = 0; i < NestingLevel; ++i)
    {
        HtmlStream << "</div>";
    }

    const std::string Html = HtmlStream.str();

    HtmlParser::Parser Parser;

    const auto StartTime = std::chrono::high_resolution_clock::now();

    // Parse the deeply nested HTML
    const HtmlParser::DOM DOM = Parser.Parse(Html);

    const auto EndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> Timer = EndTime - StartTime;

    std::cout << "Parsed deeply nested HTML with " << NestingLevel << " levels in " << Timer.count() << " seconds.\n";

    return 0;
}