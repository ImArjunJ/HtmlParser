#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace HtmlParser
{
    enum class NodeType
    {
        Document,
        Element,
        Text,
        Comment,
        Doctype,
    };

    class Node : public std::enable_shared_from_this<Node>
    {
    public:
        Node(NodeType _Type);
        ~Node();

        NodeType Type;
        std::string Tag;
        std::string Text;
        std::unordered_map<std::string, std::string> Attributes;
        std::vector<std::shared_ptr<Node>> Children;
        std::weak_ptr<Node> Parent;

        void AppendChild(const std::shared_ptr<Node>& Child);
        void InsertBefore(const std::shared_ptr<Node>& Child, const std::shared_ptr<Node>& Reference);
        bool RemoveChild(const std::shared_ptr<Node>& Child);
        void Remove();
        std::string GetAttribute(const std::string& Name) const;
        bool HasAttribute(const std::string& Name) const;
        void SetAttribute(const std::string& Name, const std::string& Value);
        bool RemoveAttribute(const std::string& Name);
        bool HasClass(const std::string& ClassName) const;
        std::string GetTextContent() const;
        void SetTextContent(const std::string& TextContent);
        std::string ToHtml() const;
        std::vector<std::shared_ptr<Node>> Select(std::string_view Pattern) const;
        std::shared_ptr<Node> SelectFirst(std::string_view Pattern) const;
        bool Matches(std::string_view Pattern) const;
        std::shared_ptr<Node> Closest(std::string_view Pattern);
    };
} // namespace HtmlParser
