#include <HtmlParser/Node.hpp>
#include <HtmlParser/Query.hpp>
#include <regex>

namespace HtmlParser
{
    // Find the first element that matches the given CSS selector
    std::shared_ptr<Node> Query::Find(const DOM& DOM, const std::string& Selector)
    {
        std::vector<std::shared_ptr<Node>> Elements;
        for (const auto& RootNode : DOM.GetRoots())
        {
            this->SearchDOM(RootNode, Selector, Elements, true);
            if (!Elements.empty())
                return Elements.front();
        }
        return nullptr;
    }

    // Find all elements that match the given CSS selector
    std::vector<std::shared_ptr<Node>> Query::FindAll(const DOM& DOM, const std::string& Selector)
    {
        std::vector<std::shared_ptr<Node>> Elements;
        for (const auto& root : DOM.GetRoots())
        {
            this->SearchDOM(root, Selector, Elements, false);
        }
        return Elements;
    }

    // Recursive function to search the DOM for matching nodes
    void Query::SearchDOM(const std::shared_ptr<Node>& RootNode, const std::string& Selector, std::vector<std::shared_ptr<Node>>& Nodes, bool FindFirst)
    {
        // Check if the current node matches the selector
        if (MatchTagName(RootNode, Selector) || MatchId(RootNode, Selector) || MatchClass(RootNode, Selector))
        {
            Nodes.push_back(RootNode);
            if (FindFirst)
                return;
        }

        // Recursively search through child nodes
        for (const auto& ChildNode : RootNode->GetChildren())
        {
            SearchDOM(ChildNode, Selector, Nodes, FindFirst);
            if (FindFirst && !Nodes.empty())
                return;
        }
    }

    // Utility function to match by tag name
    bool Query::MatchTagName(const std::shared_ptr<Node>& RootNode, const std::string& TagName)
    {
        return RootNode->GetTag() == TagName;
    }

    // Utility function to match by ID
    bool Query::MatchId(const std::shared_ptr<Node>& RootNode, const std::string& Id)
    {
        auto Attribute = RootNode->GetAttribute("id");
        return Attribute.has_value() && Attribute.value() == Id;
    }

    // Utility function to match by class name
    bool Query::MatchClass(const std::shared_ptr<Node>& RootNode, const std::string& ClassName)
    {
        auto Attribute = RootNode->GetAttribute("class");
        if (!Attribute.has_value())
            return false;

        // Check if the class name is present in the class attribute (multiple classes case)
        std::string ClassList = Attribute.value();
        std::regex ClassRegex("\\b" + ClassName + "\\b");
        return std::regex_search(ClassList, ClassRegex);
    }
} // namespace HtmlParser
