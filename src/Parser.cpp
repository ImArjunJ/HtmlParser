#include <HtmlParser/Node.hpp>
#include <HtmlParser/Parser.hpp>
#include <sstream>
#include <stack>
#include <stdexcept>

#include "Utilities.hpp"

namespace HtmlParser
{
    DOM Parser::Parse(const std::string& RawHtml)
    {
        std::stack<std::shared_ptr<Node>> NodeStack;
        auto RootNode = std::make_shared<Node>();
        RootNode->Tag = "root";
        NodeStack.push(RootNode);

        std::string CurrentTag;
        std::string CurrentText;
        bool InsideTag = false;
        bool IsClosingTag = false;
        bool IsComment = false;
        bool InsideAttributeSection = false;
        std::string AttributeBuffer;

        for (size_t i = 0; i < RawHtml.size(); ++i)
        {
            char c = RawHtml[i];

            if (c == '<')
            {
                if (i + 3 < RawHtml.size() && RawHtml.substr(i, 4) == "<!--")
                {
                    IsComment = true;
                    i += 3; // Skip comment opening
                    continue;
                }

                if (!CurrentText.empty())
                {
                    NodeStack.top()->Text += CurrentText;
                    CurrentText.clear();
                }

                InsideTag = true;
                IsClosingTag = false;
                CurrentTag.clear();
                AttributeBuffer.clear();
                InsideAttributeSection = false;
            }
            // End of a tag
            else if (c == '>')
            {
                InsideTag = false;

                if (IsComment)
                {
                    IsComment = false;
                    continue; // Ignore comment content
                }

                if (IsClosingTag)
                {
                    if (!NodeStack.empty() && NodeStack.top()->Tag == CurrentTag)
                    {
                        NodeStack.pop();
                    }
                    else if (IsStrict)
                    {
                        throw std::runtime_error("Unmatched closing tag: " + CurrentTag);
                    }
                }
                else
                {
                    auto CurrentNode = std::make_shared<Node>();
                    CurrentNode->Tag = CurrentTag;

                    if (!AttributeBuffer.empty())
                    {
                        CurrentNode->Attributes = ParseAttributes(AttributeBuffer);
                    }

                    CurrentNode->Text = Utils::Trim(CurrentNode->Text);
                    NodeStack.top()->Children.push_back(CurrentNode);
                    if (!Utils::IsSelfClosingTag(CurrentTag))
                    {
                        NodeStack.push(CurrentNode);
                    }
                }
            }
            else if (InsideTag && c == '/')
            {
                IsClosingTag = true;
            }
            else if (InsideTag)
            {
                if (std::isspace(c) && !CurrentTag.empty())
                {
                    InsideAttributeSection = true; // Start of attribute section
                }
                else if (InsideAttributeSection)
                {
                    AttributeBuffer += c;
                }
                else
                {
                    CurrentTag += c; // Collecting tag name
                }
            }
            else
            {
                CurrentText += c;
            }
        }

        if (!CurrentText.empty())
        {
            NodeStack.top()->Text += CurrentText;
        }

        if (IsStrict && NodeStack.size() > 1)
        {
            throw std::runtime_error("Unclosed tag: " + NodeStack.top()->Tag);
        }

        return DOM(RootNode->Children.empty() ? RootNode : RootNode->Children[0]);
    }

    std::unordered_map<std::string, std::string> Parser::ParseAttributes(const std::string& Input)
    {
        std::unordered_map<std::string, std::string> Attributes;
        std::istringstream Stream(Input);
        std::string Token;
        while (Stream >> Token)
        {
            size_t EqualPos = Token.find('=');
            if (EqualPos != std::string::npos)
            {
                std::string Key = Token.substr(0, EqualPos);
                std::string Value = Token.substr(EqualPos + 1);

                // Remove quotes around the value if they exist
                if (!Value.empty() && (Value.front() == '"' || Value.front() == '\''))
                {
                    Value = Value.substr(1, Value.size() - 2); // Strip leading and trailing quotes
                }

                Attributes[Utils::Trim(Key)] = Utils::Trim(Value);
            }
            else
            {
                // Handle boolean attributes (e.g., "checked", "disabled")
                Attributes[Utils::Trim(Token)] = "";
            }
        }
        return Attributes;
    }

    void Parser::SetStrict(bool Strict)
    {
        IsStrict = Strict;
    }
} // namespace HtmlParser
