#include <HtmlParser/Node.hpp>
#include <HtmlParser/Query.hpp>


namespace HtmlParser
{
    // Find an attribute by name
    std::optional<std::string> Node::GetAttribute(const std::string& Name) const
    {
        auto it = Attributes.find(Name);
        if (it != Attributes.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    // Find the first node matching a CSS selector using the Query class
    std::shared_ptr<Node> Node::Find(const std::string& Selector) const
    {
        // Create a temporary DOM with this node as the root
        DOM DOM(std::make_shared<Node>(*this));

        Query Query;
        return Query.Find(DOM, Selector);
    }

    // Find all nodes matching a CSS selector using the Query class
    std::vector<std::shared_ptr<Node>> Node::FindAll(const std::string& Selector) const
    {
        // Create a temporary DOM with this node as the root
        DOM DOM(std::make_shared<Node>(*this));

        Query Query;
        return Query.FindAll(DOM, Selector);
    }
} // namespace HtmlParser
