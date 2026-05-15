#include <HtmlParser/Node.hpp>

#include <algorithm>
#include <sstream>

namespace HtmlParser
{
    Node::Node(enum NodeType _Type) : Type(_Type)
    {
    }

    void Node::AppendChild(const std::shared_ptr<Node>& Child)
    {
        Child->Parent = shared_from_this();
        Children.push_back(Child);
    }

    bool Node::RemoveChild(const std::shared_ptr<Node>& Child)
    {
        auto it = std::find(Children.begin(), Children.end(), Child);
        if (it == Children.end())
        {
            return false;
        }

        (*it)->Parent.reset();
        Children.erase(it);
        return true;
    }

    std::string Node::GetAttribute(const std::string& Name) const
    {
        auto it = Attributes.find(Name);
        return it != Attributes.end() ? it->second : "";
    }

    std::string Node::GetTextContent() const
    {
        if (Type == NodeType::Text)
        {
            return Text;
        }
        else
        {
            std::string Result;
            for (const auto& child : Children)
            {
                Result += child->GetTextContent();
            }
            return Result;
        }
    }

    void Node::SetAttribute(const std::string& Name, const std::string& Value)
    {
        Attributes[Name] = Value;
    }

    void Node::SetTextContent(const std::string& TextContent)
    {
        if (Type == NodeType::Text || Type == NodeType::Comment || Type == NodeType::Doctype)
        {
            Text = TextContent;
            return;
        }

        for (const auto& Child : Children)
        {
            Child->Parent.reset();
        }
        Children.clear();

        if (!TextContent.empty())
        {
            auto TextNode = std::make_shared<Node>(NodeType::Text);
            TextNode->Text = TextContent;
            AppendChild(TextNode);
        }
    }

    bool Node::HasClass(const std::string& ClassName) const
    {
        auto it = Attributes.find("class");
        if (it != Attributes.end())
        {
            std::istringstream Stream(it->second);
            std::string Token;
            while (Stream >> Token)
            {
                if (Token == ClassName)
                {
                    return true;
                }
            }
        }
        return false;
    }
} // namespace HtmlParser
