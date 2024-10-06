#include <HtmlParser/Node.hpp>
#include <HtmlParser/Parser.hpp>
#include <stack>
#include <stdexcept>

namespace HtmlParser
{
    DOM Parser::Parse(const std::string& RawHtml)
    {
        std::stack<std::shared_ptr<Node>> NodeStack = {};
        auto RootNode = std::make_shared<Node>();
        RootNode->Tag = "root";
        NodeStack.push(RootNode);

        std::string CurrentTag = "";
        std::string CurrentText = "";
        bool InsideTag = false;
        bool IsClosingTag = false;
        bool IsSelfClosingTag = false;

        for (size_t i = 0; i < RawHtml.size(); ++i)
        {
            char c = RawHtml[i];

            // Start of a tag
            if (c == '<')
            {
                if (!CurrentText.empty())
                {
                    // Add collected text to the last node
                    NodeStack.top()->Text += CurrentText;
                    CurrentText.clear();
                }

                InsideTag = true;
                IsClosingTag = false;
                IsSelfClosingTag = false;
                CurrentTag.clear();
            }
            // End of a tag
            else if (c == '>')
            {
                InsideTag = false;

                if (IsClosingTag)
                {
                    // Handle closing tag
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
                    // Handle opening tag
                    auto CurrentNode = std::make_shared<Node>();
                    CurrentNode->Tag = CurrentTag;

                    // Check for self-closing tags (e.g., <br />, <img />, etc.)
                    if (IsSelfClosingTag)
                    {
                        NodeStack.top()->Children.push_back(CurrentNode);
                    }
                    else
                    {
                        NodeStack.top()->Children.push_back(CurrentNode);
                        NodeStack.push(CurrentNode);
                    }
                }
            }
            // Check for closing tag marker
            else if (InsideTag && c == '/')
            {
                IsClosingTag = true;
            }
            // Check for self-closing tag marker (e.g., <tag />)
            else if (InsideTag && c == ' ')
            {
                IsSelfClosingTag = true; // Assume it might be a self-closing tag
            }
            // Inside a tag, reading the tag name
            else if (InsideTag)
            {
                if (CurrentTag.empty() && c == '/')
                {
                    IsClosingTag = true;
                }
                else if (!IsSelfClosingTag) // Ignore any attributes or unnecessary spaces
                {
                    CurrentTag += c;
                }
            }
            else
            {
                CurrentText += c;
            }
        }

        // Final text check
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

    void Parser::SetStrict(bool Strict)
    {
        IsStrict = Strict;
    }
} // namespace HtmlParser
