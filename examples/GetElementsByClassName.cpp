#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML with multiple elements sharing the same class
    std::string Html = R"(
        <ul>
            <li class="item">Item 1</li>
            <li class="item">Item 2</li>
            <li class="item">Item 3</li>
        </ul>
    )";

    HtmlParser::Parser Parser;
    const HtmlParser::DOM DOM = Parser.Parse(Html);

    // Get all elements with class "item"
    const auto Items = DOM.GetElementsByClassName("item");

    std::cout << "Found " << Items.size() << " elements with class 'item':\n";

    // Output the text content of each item
    for (const auto& Item : Items)
    {
        std::cout << "- " << Item->GetTextContent() << "\n";
    }

    return 0;
}