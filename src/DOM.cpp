#include <HtmlParser/DOM.hpp>
#include <unordered_set>

#include "Utilities.hpp"

namespace HtmlParser
{
    DOM::DOM(const std::shared_ptr<Node>& Root) : Document(Root)
    {
    }

    std::shared_ptr<Node> DOM::Root() const
    {
        return Document;
    }

    void DOM::Traverse(const std::function<void(const std::shared_ptr<Node>&)>& Visitor) const
    {
        TraverseImpl(Document, Visitor);
    }

    void DOM::TraverseImpl(const std::shared_ptr<Node>& ElementNode, const std::function<void(const std::shared_ptr<Node>&)>& Visitor) const
    {
        Visitor(ElementNode);
        for (const auto& child : ElementNode->Children)
        {
            TraverseImpl(child, Visitor);
        }
    }

    std::vector<std::shared_ptr<Node>> DOM::GetElementsByTagName(const std::string& TagName) const
    {
        std::vector<std::shared_ptr<Node>> Elements;
        GetElementsByTagNameImpl(Document, TagName, Elements);
        return Elements;
    }

    void DOM::GetElementsByTagNameImpl(const std::shared_ptr<Node>& ElementNode, const std::string& TagName, std::vector<std::shared_ptr<Node>>& Elements) const
    {
        if (ElementNode->NodeType == NodeType::Element && Utils::ToLower(ElementNode->Tag) == Utils::ToLower(TagName))
        {
            Elements.push_back(ElementNode);
        }
        for (const auto& child : ElementNode->Children)
        {
            GetElementsByTagNameImpl(child, TagName, Elements);
        }
    }

    std::vector<std::shared_ptr<Node>> DOM::GetElementsByClassName(const std::string& ClassName) const
    {
        std::vector<std::shared_ptr<Node>> Elements;
        GetElementsByClassNameImpl(Document, ClassName, Elements);
        return Elements;
    }

    void DOM::GetElementsByClassNameImpl(const std::shared_ptr<Node>& ElementNode, const std::string& ClassName, std::vector<std::shared_ptr<Node>>& Elements) const
    {
        if (ElementNode->NodeType == NodeType::Element && ElementNode->HasClass(ClassName))
        {
            Elements.push_back(ElementNode);
        }
        for (const auto& Child : ElementNode->Children)
        {
            GetElementsByClassNameImpl(Child, ClassName, Elements);
        }
    }

    std::shared_ptr<Node> DOM::GetElementById(const std::string& Id) const
    {
        std::shared_ptr<Node> Result = nullptr;
        GetElementByIdImpl(Document, Id, Result);
        return Result;
    }

    void DOM::GetElementByIdImpl(const std::shared_ptr<Node>& ElementNode, const std::string& Id, std::shared_ptr<Node>& Result) const
    {
        if (Result)
            return;

        if (ElementNode->NodeType == NodeType::Element)
        {
            auto it = ElementNode->Attributes.find("id");
            if (it != ElementNode->Attributes.end() && it->second == Id)
            {
                Result = ElementNode;
                return;
            }
        }
        for (const auto& Child : ElementNode->Children)
        {
            GetElementByIdImpl(Child, Id, Result);
            if (Result)
                return;
        }
    }

    std::string DOM::ToHtml() const
    {
        std::string Html;
        for (const auto& Child : Document->Children)
        {
            ToHtmlImpl(Child, Html);
        }
        return Html;
    }

    void DOM::ToHtmlImpl(const std::shared_ptr<Node>& ElementNode, std::string& Html) const
    {
        static const std::unordered_set<std::string> VoidElements = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "param", "source", "track", "wbr"};

        switch (ElementNode->NodeType)
        {
        case NodeType::Element:
        {
            Html += "<" + ElementNode->Tag;

            // Add attributes
            for (const auto& Attribute : ElementNode->Attributes)
            {
                Html += " " + Attribute.first + "=\"" + Utils::EscapeHtml(Attribute.second) + "\"";
            }

            if (VoidElements.count(Utils::ToLower(ElementNode->Tag)) > 0)
            {
                // Void element, no closing tag
                Html += ">";
            }
            else
            {
                Html += ">";
                // Recursively serialize child nodes
                for (const auto& Child : ElementNode->Children)
                {
                    ToHtmlImpl(Child, Html);
                }
                Html += "</" + ElementNode->Text + ">";
            }
            break;
        }
        case NodeType::Text:
        {
            Html += Utils::EscapeHtml(ElementNode->Text);
            break;
        }
        case NodeType::Comment:
        {
            Html += "<!--" + ElementNode->Text + "-->";
            break;
        }
        case NodeType::Doctype:
        {
            Html += "<!DOCTYPE " + ElementNode->Text + ">";
            break;
        }
        case NodeType::Document:
        {
            // Should not reach here
            break;
        }
        }
    }
} // namespace HtmlParser