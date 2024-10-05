#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Node.hpp"

namespace HtmlParser
{
    class DOM
    {
    public:
        explicit DOM(std::shared_ptr<Node> root) : m_Root(root)
        {
        }

        // Get the root node of the DOM
        std::shared_ptr<Node> Root() const
        {
            return m_Root;
        }

        std::vector<std::shared_ptr<Node>> GetRoots() const;

        // Serialize the DOM back into HTML
        std::string ToHtml() const;

        // Find the first node matching the given CSS selector using the Query class
        std::shared_ptr<Node> Find(const std::string& selector) const;

        // Find all nodes matching the given CSS selector using the Query class
        std::vector<std::shared_ptr<Node>> FindAll(const std::string& selector) const;

    private:
        std::shared_ptr<Node> m_Root; // Root of the DOM tree
    };
} // namespace HtmlParser
