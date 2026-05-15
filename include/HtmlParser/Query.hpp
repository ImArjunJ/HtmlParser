#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Node.hpp"

namespace HtmlParser
{

    class Query
    {
    public:
        Query(const std::shared_ptr<Node>& QueryRoot);

        std::vector<std::shared_ptr<Node>> Select(const std::string& Selector) const;
        std::shared_ptr<Node> SelectFirst(const std::string& Selector) const;

    private:
        std::shared_ptr<Node> m_Root;

        std::vector<std::string> TokenizeSelector(const std::string& Selector) const;

        bool MatchSelector(const std::shared_ptr<Node>& ElementNode, const std::string& Token) const;
    };

} // namespace HtmlParser
