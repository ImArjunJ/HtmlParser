#include <HtmlParser/Parser.hpp>
#include <chrono>
#include <iostream>

int main()
{
    const std::uint32_t ParseCount = 100000;
    HtmlParser::Parser Parser;
    const std::string Html = "<html><body><p>Hello World</p></body></html>";

    const auto StartTime = std::chrono::high_resolution_clock::now();

    for (std::uint32_t i = 0; i < ParseCount; ++i)
    {
        Parser.Parse(Html);
    }

    const auto EndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> Timer = EndTime - StartTime;

    std::cout << "Parsed simple HTML " << ParseCount << " times in " << Timer.count() << " seconds.\n";
    std::cout << "Average time per parse: " << (Timer.count() / ParseCount) * 1e6 << " microseconds.\n";

    return 0;
}