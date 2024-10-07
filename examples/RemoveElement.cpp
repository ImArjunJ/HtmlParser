#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML with a list of items
    std::string Html = R"(
        <ul>
            <li>First Item</li>
            <li id="remove-me">Item to Remove</li>
            <li>Last Item</li>
        </ul>
    )";

    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    // Get the <li> element to remove
    auto ItemToRemove = DOM.GetElementById("remove-me");

    if (ItemToRemove)
    {
        // Get the parent node
        auto Parent = ItemToRemove->Parent.lock();
        if (Parent)
        {
            // Remove the element from its parent's children
            auto& Siblings = Parent->Children;
            Siblings.erase(std::remove(Siblings.begin(), Siblings.end(), ItemToRemove), Siblings.end());
            std::cout << "Element removed.\n";
        }
    }
    else
    {
        std::cout << "Element to remove not found.\n";
    }

    // Serialize and output the modified HTML
    std::cout << "Modified HTML:\n" << DOM.ToHtml() << "\n";

    return 0;
}