#include <HtmlParser/DOM.hpp>
#include <chrono>
#include <iostream>


int main()
{
    const std::uint32_t NodeCount = 10000;
    auto StartTime = std::chrono::high_resolution_clock::now();

    auto RootNode = std::make_shared<HtmlParser::Node>();
    RootNode->Tag = "html";

    for (std::uint32_t i = 0; i < NodeCount; ++i)
    {
        auto p = std::make_shared<HtmlParser::Node>();
        p->Tag = "p";
        p->Text = "Paragraph " + std::to_string(i);
        RootNode->Children.push_back(p);
    }

    HtmlParser::DOM DOM(RootNode);
    auto EndTime = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> Timer = EndTime - StartTime;
    std::cout << "Created DOM with " << NodeCount << " nodes in " << Timer.count() << " seconds." << std::endl;

    // Benchmark querying
    StartTime = std::chrono::high_resolution_clock::now();
    DOM.FindAll("p");
    EndTime = std::chrono::high_resolution_clock::now();
    Timer = EndTime - StartTime;

    std::cout << "Querying took " << Timer.count() << " seconds." << std::endl;

    return 0;
}
