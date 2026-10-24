#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Query.hpp>

#include "Tree.hpp"
#include "Utilities.hpp"

namespace HtmlParser
{
    DOM::DOM() : m_Document(std::make_shared<Node>(NodeType::Document)) {}
    DOM::DOM(const std::shared_ptr<Node>& Root) : m_Document(Root) {}

    std::shared_ptr<Node> DOM::Root() const
    {
        return m_Document;
    }

    void DOM::Traverse(const std::function<void(const std::shared_ptr<Node>&)>& Visitor) const
    {
        if (!m_Document || !Visitor)
            return;
        // Keep pending nodes alive when the callback removes or reparents them.
        std::vector<std::shared_ptr<Node>> Pending{m_Document};
        while (!Pending.empty())
        {
            auto Current = std::move(Pending.back());
            Pending.pop_back();
            if (!Current)
                continue;
            Visitor(Current);
            Pending.insert(Pending.end(), Current->Children.rbegin(), Current->Children.rend());
        }
    }

    std::vector<std::shared_ptr<Node>> DOM::GetElementsByTagName(const std::string& TagName) const
    {
        const auto Normalized = Utils::ToLower(TagName);
        std::vector<std::shared_ptr<Node>> Elements;
        Detail::Walk(m_Document.get(),
                     [&](const auto& Current)
                     {
                         if (Current->Type == NodeType::Element &&
                             (Normalized == "*" || Utils::EqualsIgnoreCase(Current->Tag, Normalized)))
                             Elements.push_back(Current->shared_from_this());
                         return true;
                     });
        return Elements;
    }

    std::vector<std::shared_ptr<Node>> DOM::GetElementsByClassName(const std::string& ClassName) const
    {
        const auto Classes = Utils::Words(ClassName);
        std::vector<std::shared_ptr<Node>> Elements;
        if (Classes.empty())
            return Elements;
        Detail::Walk(m_Document.get(),
                     [&](const auto& Current)
                     {
                         if (Current->Type == NodeType::Element &&
                             std::all_of(Classes.begin(), Classes.end(),
                                         [&](auto Class)
                                         {
                                             const auto It = Current->Attributes.find("class");
                                             return It != Current->Attributes.end() &&
                                                    Utils::ContainsWord(It->second, Class);
                                         }))
                             Elements.push_back(Current->shared_from_this());
                         return true;
                     });
        return Elements;
    }

    std::shared_ptr<Node> DOM::GetElementById(const std::string& Id) const
    {
        std::shared_ptr<Node> Result;
        Detail::Walk(m_Document.get(),
                     [&](const auto& Current)
                     {
                         const auto It = Current->Attributes.find("id");
                         if (Current->Type == NodeType::Element && It != Current->Attributes.end() && It->second == Id)
                             Result = Current->shared_from_this();
                         return !Result;
                     });
        return Result;
    }

    std::vector<std::shared_ptr<Node>> DOM::Select(std::string_view Pattern) const
    {
        return Query(m_Document).Select(Pattern);
    }

    std::shared_ptr<Node> DOM::SelectFirst(std::string_view Pattern) const
    {
        return Query(m_Document).SelectFirst(Pattern);
    }

    std::string DOM::ToHtml() const
    {
        return m_Document ? Detail::Serialize(*m_Document) : std::string{};
    }

    namespace Detail
    {
        std::string Serialize(const Node& Root)
        {
            struct Frame
            {
                const Node* Current;
                bool Closing;
                bool RawText;
            };
            std::vector<Frame> Pending{{&Root, false, false}};
            std::string Html;
            while (!Pending.empty())
            {
                const auto [Current, Closing, RawText] = Pending.back();
                Pending.pop_back();
                if (!Current)
                    continue;
                if (Closing)
                {
                    Html += "</";
                    Html += Current->Tag;
                    Html += '>';
                    continue;
                }
                switch (Current->Type)
                {
                case NodeType::Text:
                    if (RawText)
                        Html += Current->Text;
                    else
                        Utils::AppendEscaped(Html, Current->Text);
                    continue;
                case NodeType::Comment:
                    Html += "<!--";
                    Html += Current->Text;
                    Html += "-->";
                    continue;
                case NodeType::Doctype:
                    Html += "<!DOCTYPE ";
                    Html += Current->Text;
                    Html += '>';
                    continue;
                case NodeType::Element:
                {
                    Html += '<';
                    Html += Current->Tag;
                    std::vector<const std::pair<const std::string, std::string>*> Attributes;
                    Attributes.reserve(Current->Attributes.size());
                    for (const auto& Attribute : Current->Attributes)
                        Attributes.push_back(&Attribute);
                    std::sort(Attributes.begin(), Attributes.end(),
                              [](auto Left, auto Right)
                              {
                                  return Left->first < Right->first;
                              });
                    for (const auto* Attribute : Attributes)
                    {
                        Html += ' ';
                        Html += Attribute->first;
                        Html += "=\"";
                        Utils::AppendEscaped(Html, Attribute->second);
                        Html += '"';
                    }
                    Html += '>';
                    if (Utils::IsVoidElement(Current->Tag))
                        continue;
                    if ((Utils::EqualsIgnoreCase(Current->Tag, "pre") ||
                         Utils::EqualsIgnoreCase(Current->Tag, "textarea") ||
                         Utils::EqualsIgnoreCase(Current->Tag, "listing")) &&
                        !Current->Children.empty() && Current->Children.front() &&
                        Current->Children.front()->Type == NodeType::Text &&
                        Current->Children.front()->Text.starts_with('\n'))
                        Html += '\n';
                    Pending.push_back({Current, true, false});
                    break;
                }
                case NodeType::Document:
                    break;
                }
                const bool ChildRawText = Current->Type == NodeType::Element && Utils::IsRawTextElement(Current->Tag);
                for (auto It = Current->Children.rbegin(); It != Current->Children.rend(); ++It)
                    Pending.push_back({It->get(), false, ChildRawText});
            }
            return Html;
        }
    } // namespace Detail
} // namespace HtmlParser
