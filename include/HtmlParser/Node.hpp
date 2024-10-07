#pragma once
#include <memory>
#include <string>
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
        Node(NodeType Type);

        HtmlParser::NodeType NodeType;
        std::string Tag;
        std::string Text;
        std::unordered_map<std::string, std::string> Attributes;
        std::vector<std::shared_ptr<Node>> Children;
        std::weak_ptr<Node> Parent;

        void AppendChild(const std::shared_ptr<Node>& Child);
        std::string GetAttribute(const std::string& Name) const;
        void SetAttribute(const std::string& Name, const std::string& Value);
        bool HasClass(const std::string& ClassName) const;
        std::string GetTextContent() const;
    };
} // namespace HtmlParser
