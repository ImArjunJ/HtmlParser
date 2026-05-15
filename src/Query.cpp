#include <HtmlParser/Query.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <unordered_set>
#include <utility>

#include "Utilities.hpp"

namespace HtmlParser
{
    namespace
    {
        bool IsCombinator(const std::string& Token)
        {
            return Token == " " || Token == ">" || Token == "+" || Token == "~";
        }

        void AddUnique(const std::shared_ptr<Node>& ElementNode, std::vector<std::shared_ptr<Node>>& Results, std::unordered_set<const Node*>& Seen)
        {
            if (Seen.insert(ElementNode.get()).second)
            {
                Results.push_back(ElementNode);
            }
        }
    } // namespace

    Query::Query(const std::shared_ptr<Node>& QueryRoot) : m_Root(QueryRoot)
    {
    }

    std::vector<std::shared_ptr<Node>> Query::Select(const std::string& Selector) const
    {
        std::vector<std::string> Tokens = TokenizeSelector(Selector);
        if (Tokens.empty() || IsCombinator(Tokens.front()))
        {
            return {};
        }

        std::vector<std::shared_ptr<Node>> CurrentMatches;
        std::unordered_set<const Node*> Seen;

        std::function<void(const std::shared_ptr<Node>&)> CollectInitialMatches = [&](const std::shared_ptr<Node>& Node) {
            if (MatchSelector(Node, Tokens.front()))
            {
                AddUnique(Node, CurrentMatches, Seen);
            }

            for (const auto& Child : Node->Children)
            {
                CollectInitialMatches(Child);
            }
        };

        CollectInitialMatches(m_Root);

        for (size_t Index = 1; Index < Tokens.size(); Index += 2)
        {
            if (Index + 1 >= Tokens.size() || !IsCombinator(Tokens[Index]) || IsCombinator(Tokens[Index + 1]))
            {
                return {};
            }

            const std::string& Combinator = Tokens[Index];
            const std::string& SimpleSelector = Tokens[Index + 1];
            std::vector<std::shared_ptr<Node>> NextMatches;
            std::unordered_set<const Node*> NextSeen;

            auto AddIfMatching = [&](const std::shared_ptr<Node>& Node) {
                if (MatchSelector(Node, SimpleSelector))
                {
                    AddUnique(Node, NextMatches, NextSeen);
                }
            };

            std::function<void(const std::shared_ptr<Node>&)> CollectDescendants = [&](const std::shared_ptr<Node>& Node) {
                for (const auto& Child : Node->Children)
                {
                    AddIfMatching(Child);
                    CollectDescendants(Child);
                }
            };

            for (const auto& Match : CurrentMatches)
            {
                if (Combinator == " ")
                {
                    CollectDescendants(Match);
                }
                else if (Combinator == ">")
                {
                    for (const auto& Child : Match->Children)
                    {
                        AddIfMatching(Child);
                    }
                }
                else if (Combinator == "+" || Combinator == "~")
                {
                    auto Parent = Match->Parent.lock();
                    if (!Parent)
                    {
                        continue;
                    }

                    auto it = std::find(Parent->Children.begin(), Parent->Children.end(), Match);
                    if (it == Parent->Children.end())
                    {
                        continue;
                    }

                    if (Combinator == "+")
                    {
                        for (++it; it != Parent->Children.end(); ++it)
                        {
                            if ((*it)->Type == NodeType::Element)
                            {
                                AddIfMatching(*it);
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (++it; it != Parent->Children.end(); ++it)
                        {
                            if ((*it)->Type == NodeType::Element)
                            {
                                AddIfMatching(*it);
                            }
                        }
                    }
                }
            }

            CurrentMatches = std::move(NextMatches);
        }

        return CurrentMatches;
    }

    std::shared_ptr<Node> Query::SelectFirst(const std::string& Selector) const
    {
        std::vector<std::shared_ptr<Node>> Results = Select(Selector);
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
            if (std::isspace(static_cast<unsigned char>(c)))
            {
                if (!Token.empty())
                {
                    Tokens.push_back(Token);
                    Token.clear();
                }

                size_t Next = i + 1;
                while (Next < Selector.size() && std::isspace(static_cast<unsigned char>(Selector[Next])))
                {
                    ++Next;
                }

                if (Next < Selector.size() && Selector[Next] != '>' && Selector[Next] != '+' && Selector[Next] != '~' && !Tokens.empty() && !IsCombinator(Tokens.back()))
                {
                    Tokens.push_back(" ");
                }
                i = Next - 1;
            }
            else if (c == '>' || c == '+' || c == '~')
            {
                if (!Token.empty())
                {
                    Tokens.push_back(Token);
                    Token.clear();
                }
                if (!Tokens.empty() && Tokens.back() == " ")
                {
                    Tokens.pop_back();
                }
                Tokens.emplace_back(1, c);

                while (i + 1 < Selector.size() && std::isspace(static_cast<unsigned char>(Selector[i + 1])))
                {
                    ++i;
                }
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
                if (TagName != "*" && Utils::ToLower(ElementNode->Tag) != Utils::ToLower(TagName))
                {
                    IsMatching = false;
                }
            }
        }
        return IsMatching;
    }
} // namespace HtmlParser
