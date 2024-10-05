#include <HtmlParser/Parser.hpp>
#include <chrono>
#include <iostream>


int main()
{
    const std::uint32_t ParseCount = 10000;
    HtmlParser::Parser Parser;

    auto StartTime = std::chrono::high_resolution_clock::now();
    for (std::uint32_t i = 0; i < ParseCount; ++i)
    {
        Parser.Parse("<html><body><p>Hello World</p></body></html>");
    }
    auto EndTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> Timer = EndTime - StartTime;
    std::cout << "Parsed " << ParseCount << " times in " << Timer.count() << " seconds." << std::endl;

    return 0;
}
