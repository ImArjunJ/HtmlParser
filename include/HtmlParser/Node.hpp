#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace HtmlParser
{
    struct Node
    {
        std::string Tag;
        std::unordered_map<std::string, std::string> Attributes;
        std::vector<std::shared_ptr<Node>> Children;
        std::string Text;

        // Return the tag name of the node
        std::string GetTag() const
        {
            return Tag;
        }

        // Return an attribute by name (optional, in case attribute is missing)
        std::optional<std::string> GetAttribute(const std::string& Name) const;

        // Return all children of this node
        std::vector<std::shared_ptr<Node>> GetChildren() const
        {
            return Children;
        }

        // Find the first child node matching the given CSS selector using Query
        std::shared_ptr<Node> Find(const std::string& Selector) const;

        // Find all child nodes matching the given CSS selector using Query
        std::vector<std::shared_ptr<Node>> FindAll(const std::string& Selector) const;
    };
} // namespace HtmlParser
