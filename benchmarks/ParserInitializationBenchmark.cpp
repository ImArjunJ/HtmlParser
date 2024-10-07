#include <HtmlParser/Parser.hpp>
#include <chrono>
#include <iostream>

int main()
{
    const std::size_t ParserCount = 100000;

    const auto StartTime = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < ParserCount; ++i)
    {
        const HtmlParser::Parser Parser;
    }

    const auto EndTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> Timer = EndTime - StartTime;

    std::cout << "Created and destroyed " << ParserCount << " parser instances in " << Timer.count() << " seconds.\n";
    std::cout << "Average time per parser instance: " << (Timer.count() / ParserCount) * 1e6 << " microseconds.\n";

    return 0;
}