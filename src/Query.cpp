#include <HtmlParser/Query.hpp>

#include "Utilities.hpp"

namespace HtmlParser
{
    Query::Query(const std::shared_ptr<Node>& QueryRoot) : Root(QueryRoot)
    {
    }

    std::vector<std::shared_ptr<Node>> Query::Select(const std::string& Selector) const
    {
        std::vector<std::string> Tokens = TokenizeSelector(Selector);
        std::vector<std::shared_ptr<Node>> Results;
        SelectImpl(Root, Tokens, 0, Results);
        return Results;
    }

    std::shared_ptr<Node> Query::SelectFirst(const std::string& Selector) const
    {
        std::vector<std::string> Tokens = TokenizeSelector(Selector);
        std::vector<std::shared_ptr<Node>> Results;
        SelectImpl(Root, Tokens, 0, Results);
        if (!Results.empty())
        {
            return Results[0];
        }
        return nullptr;
    }

    std::vector<std::string> Query::TokenizeSelector(const std::string& Selector) const
    {
        std::vector<std::string> Tokens;
        std::string Token;
        for (size_t i = 0; i < Selector.size(); ++i)
        {
            char c = Selector[i];
            if (isspace(c))
            {
                if (!Token.empty())
                {
                    Tokens.push_back(Token);
                    Token.clear();
                }
                Tokens.push_back(" ");
            }
            else if (c == '>' || c == '+' || c == '~')
            {
                if (!Token.empty())
                {
                    Tokens.push_back(Token);
                    Token.clear();
                }
                Tokens.emplace_back(1, c);
            }
            else
            {
                Token += c;
            }
        }
        if (!Token.empty())
        {
            Tokens.push_back(Token);
        }
        return Tokens;
    }

    void Query::SelectImpl(const std::shared_ptr<Node>& ElementNode, const std::vector<std::string>& Tokens, size_t Index, std::vector<std::shared_ptr<Node>>& Results) const
    {
        if (Index >= Tokens.size())
            return;

        if (Tokens[Index] == " ")
        {
            // Descendant combinator
            for (const auto& child : ElementNode->Children)
            {
                SelectImpl(child, Tokens, Index + 1, Results);
                SelectImpl(child, Tokens, Index, Results);
            }
        }
        else
        {
            if (MatchSelector(ElementNode, Tokens[Index]))
            {
                if (Index == Tokens.size() - 1)
                {
                    Results.push_back(ElementNode);
                }
                else
                {
                    for (const auto& Child : ElementNode->Children)
                    {
                        SelectImpl(Child, Tokens, Index + 1, Results);
                    }
                }
            }
            else
            {
                for (const auto& Child : ElementNode->Children)
                {
                    SelectImpl(Child, Tokens, Index, Results);
                }
            }
        }
    }

    bool Query::MatchSelector(const std::shared_ptr<Node>& ElementNode, const std::string& Token) const
    {
        if (ElementNode->Type != NodeType::Element)
            return false;

        size_t Position = 0;
        bool IsMatching = true;

        while (Position < Token.size() && IsMatching)
        {
            if (Token[Position] == '.')
            {
                // Class selector
                ++Position;
                size_t Start = Position;
                while (Position < Token.size() && Token[Position] != '.' && Token[Position] != '#' && Token[Position] != '[')
                {
                    ++Position;
                }
                std::string ClassName = Token.substr(Start, Position - Start);
                if (!ElementNode->HasClass(ClassName))
                {
                    IsMatching = false;
                }
            }
            else if (Token[Position] == '#')
            {
                // ID selector
                ++Position;
                size_t Start = Position;
                while (Position < Token.size() && Token[Position] != '.' && Token[Position] != '#' && Token[Position] != '[')
                {
                    ++Position;
                }
                std::string Id = Token.substr(Start, Position - Start);
                if (ElementNode->GetAttribute("id") != Id)
                {
                    IsMatching = false;
                }
            }
            else if (Token[Position] == '[')
            {
                // Attribute selector
                size_t Start = Position + 1;
                size_t End = Token.find(']', Start);
                if (End == std::string::npos)
                {
                    IsMatching = false;
                    break;
                }
                std::string AttrSelector = Token.substr(Start, End - Start);
                Position = End + 1;

                size_t EqualPos = AttrSelector.find('=');
                if (EqualPos != std::string::npos)
                {
                    std::string AttrName = AttrSelector.substr(0, EqualPos);
                    std::string AttrValue = AttrSelector.substr(EqualPos + 1);

                    // Remove quotes if present
                    if (!AttrValue.empty() && (AttrValue.front() == '"' || AttrValue.front() == '\''))
                    {
                        AttrValue = AttrValue.substr(1, AttrValue.size() - 2);
                    }

                    std::string NodeAttrValue = ElementNode->GetAttribute(AttrName);
                    if (NodeAttrValue != AttrValue)
                    {
                        IsMatching = false;
                    }
                }
                else
                {
                    // Attribute existence selector
                    if (ElementNode->Attributes.find(AttrSelector) == ElementNode->Attributes.end())
                    {
                        IsMatching = false;
                    }
                }
            }
            else
            {
                // Tag selector
                size_t Start = Position;
                while (Position < Token.size() && Token[Position] != '.' && Token[Position] != '#' && Token[Position] != '[')
                {
                    ++Position;
                }
                std::string TagName = Token.substr(Start, Position - Start);
                if (Utils::ToLower(ElementNode->Tag) != Utils::ToLower(TagName))
                {
                    IsMatching = false;
                }
            }
        }
        return IsMatching;
    }
} // namespace HtmlParser
