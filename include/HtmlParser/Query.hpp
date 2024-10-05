#pragma once
#include <memory>
#include <string>
#include <vector>

#include "DOM.hpp"

namespace HtmlParser
{
    class Query
    {
    public:
        // Find the first element that matches the given CSS selector
        std::shared_ptr<Node> Find(const DOM& DOM, const std::string& Selector);

        // Find all elements that match the given CSS selector
        std::vector<std::shared_ptr<Node>> FindAll(const DOM& DOM, const std::string& Selector);

    private:
        // Utility functions to handle different types of selectors
        bool MatchTagName(const std::shared_ptr<Node>& RootNode, const std::string& TagName);
        bool MatchId(const std::shared_ptr<Node>& RootNode, const std::string& Id);
        bool MatchClass(const std::shared_ptr<Node>& RootNode, const std::string& ClassName);

        // Recursive function to search the DOM for matching nodes
        void SearchDOM(const std::shared_ptr<Node>& RootNode, const std::string& Selector, std::vector<std::shared_ptr<Node>>& Nodes, bool FindFirst);
    };
} // namespace HtmlParser
