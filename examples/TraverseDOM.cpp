#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // Sample HTML content
    const std::string Html = R"(
        <div>
            <h1>Title</h1>
            <p>Paragraph 1</p>
            <p>Paragraph 2</p>
            <span>Additional content</span>
        </div>
    )";

    HtmlParser::Parser Parser;
    const HtmlParser::DOM DOM = Parser.Parse(Html);

    int ElementCount = 0;

    // Traverse the DOM to count elements
    DOM.Traverse([&](const std::shared_ptr<HtmlParser::Node>& Node)
    {
        if (Node->Type == HtmlParser::NodeType::Element)
        {
            std::cout << "Element: " << Node->Tag << "\n";
            ++ElementCount;
        }
    });

    std::cout << "Total number of elements: " << ElementCount << "\n";
    return 0;
}
