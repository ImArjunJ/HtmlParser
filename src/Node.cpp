#include <HtmlParser/Node.hpp>
#include <HtmlParser/Query.hpp>
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
