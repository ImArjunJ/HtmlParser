#include <HtmlParser/DOM.hpp>
#include <iostream>
#include <memory>


int main()
{
    // Creating a simple DOM tree with multiple root nodes
    auto RootNode = std::make_shared<HtmlParser::Node>();
    RootNode->Tag = "html";

    auto Body1 = std::make_shared<HtmlParser::Node>();
    Body1->Tag = "body";

    auto Body2 = std::make_shared<HtmlParser::Node>();
    Body2->Tag = "body";

    RootNode->Children.push_back(Body1);
    RootNode->Children.push_back(Body2);

    // Create DOM instance
    HtmlParser::DOM DOM(RootNode);

    // Getting root nodes
    auto Roots = DOM.GetRoots();
    std::cout << "Found " << Roots.size() << " root nodes." << std::endl;

    for (const auto& RootNode : Roots)
    {
        std::cout << "Root tag: " << RootNode->Tag << std::endl;
    }

    std::cout << "Serialized HTML: " << DOM.ToHtml() << std::endl;

    return 0;
}
