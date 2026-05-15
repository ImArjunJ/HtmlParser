#include <HtmlParser/Node.hpp>
#include <HtmlParser/Parser.hpp>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

#include "Utilities.hpp"

namespace HtmlParser
{
    Parser::Parser()
    {
    }

    DOM Parser::Parse(const std::string& Input)
    {
        Tokenizer Instance(Input);
        Instance.Tokenize();
        const auto& Tokens = Instance.GetTokens();

        Document = std::make_shared<Node>(NodeType::Document);
        OpenElements.clear();
        OpenElements.push_back(Document);
        InsertionMode = InsertionMode::Initial;

        for (const auto& Token : Tokens)
        {
            if (Token.Type == TokenType::Comment)
            {
                InsertComment(Token);
                continue;
            }

            switch (InsertionMode)
            {
            case InsertionMode::Initial:
                InsertionModeInitial(Token);
                break;
            case InsertionMode::BeforeHtml:
                InsertionModeBeforeHtml(Token);
                break;
            case InsertionMode::BeforeHead:
                InsertionModeBeforeHead(Token);
                break;
            case InsertionMode::InHead:
                InsertionModeInHead(Token);
                break;
            case InsertionMode::AfterHead:
                InsertionModeAfterHead(Token);
                break;
            case InsertionMode::InBody:
                InsertionModeInBody(Token);
                break;
            default:
                HandleError("Unsupported insertion mode");
                break;
            }
        }

        if (m_IsStrict && HasOpenNonImpliedElements())
        {
            HandleError("Unclosed element: " + OpenElements.back()->Tag);
        }

        return DOM(Document);
    }

    void Parser::HandleError(const std::string& ErrorMessage)
    {
        if (m_IsStrict)
        {
            throw std::runtime_error("Parse error: " + ErrorMessage);
        }
        else
        {
            // For non-  mode, can log the error or ignore it
            // std::cerr << "Parse error: " << message << std::endl;
        }
    }

    std::shared_ptr<Node> Parser::CurrentNode()
    {
        return OpenElements.back();
    }

    void Parser::InsertDoctype(const Token& Token)
    {
        auto Doctype = std::make_shared<Node>(NodeType::Doctype);
        Doctype->Text = Token.Data.empty() ? "html" : Token.Data;
        Document->AppendChild(Doctype);
    }

    void Parser::InsertComment(const Token& Token)
    {
        auto Comment = std::make_shared<Node>(NodeType::Comment);
        Comment->Text = Token.Data;
        CurrentNode()->AppendChild(Comment);
    }

    void Parser::InsertElement(const Token& Token)
    {
        auto Element = std::make_shared<Node>(NodeType::Element);
        Element->Tag = Utils::ToLower(Token.Data);
        Element->Attributes = Token.Attributes;
        CurrentNode()->AppendChild(Element);
        if (!Token.SelfClosing && !IsVoidElement(Element->Tag))
        {
            OpenElements.push_back(Element);
        }
    }

    void Parser::InsertCharacter(const Token& Token)
    {
        if (Token.Data.empty())
        {
            return;
        }

        if (!CurrentNode()->Children.empty())
        {
            auto LastChild = CurrentNode()->Children.back();
            if (LastChild->Type == NodeType::Text)
            {
                LastChild->Text += Token.Data;
                return;
            }
        }

        auto TextNode = std::make_shared<Node>(NodeType::Text);
        TextNode->Text = Token.Data;
        CurrentNode()->AppendChild(TextNode);
    }

    void Parser::CloseElement(const Token& Token)
    {
        std::string TagName = Utils::ToLower(Token.Data);
        for (auto it = OpenElements.rbegin(); it != OpenElements.rend(); ++it)
        {
            if ((*it)->Tag == TagName)
            {
                if (m_IsStrict && it != OpenElements.rbegin())
                {
                    HandleError("Unexpected end tag: " + Token.Data);
                }
                OpenElements.erase(it.base() - 1, OpenElements.end());
                return;
            }
        }
        // If we didn't find the tag to close
        HandleError("No matching start tag for end tag: " + Token.Data);
    }

    bool Parser::HasOpenNonImpliedElements() const
    {
        for (const auto& Element : OpenElements)
        {
            if (Element->Type != NodeType::Element)
            {
                continue;
            }

            const std::string TagName = Utils::ToLower(Element->Tag);
            if (TagName != "html" && TagName != "head" && TagName != "body")
            {
                return true;
            }
        }
        return false;
    }

    bool Parser::IsVoidElement(const std::string& TagName) const
    {
        static const std::unordered_set<std::string> VoidElements = {
            "area", "base", "br", "col", "embed", "hr", "img", "input",
            "link", "meta", "param", "source", "track", "wbr"};
        return VoidElements.count(Utils::ToLower(TagName)) > 0;
    }

    void Parser::InsertionModeInitial(const Token& Token)
    {
        if (Token.Type == TokenType::DOCTYPE)
        {
            InsertDoctype(Token);
        }
        else
        {
            InsertionMode = InsertionMode::BeforeHtml;
            InsertionModeBeforeHtml(Token);
        }
    }

    void Parser::InsertionModeBeforeHtml(const Token& Token)
    {
        if (Token.Type == TokenType::Character && std::isspace(static_cast<unsigned char>(Token.Data[0]))) {
            return; // Ignore whitespace
        }
        if (Token.Type == TokenType::StartTag && Utils::ToLower(Token.Data) == "html")
        {
            InsertElement(Token);
            InsertionMode = InsertionMode::BeforeHead;
        }
        else
        {
            // Implicitly create <html>
            HtmlParser::Token HtmlToken;
            HtmlToken.Type = TokenType::StartTag;
            HtmlToken.Data = "html";
            InsertElement(HtmlToken);
            InsertionMode = InsertionMode::BeforeHead;
            InsertionModeBeforeHead(Token);
        }
    }

    void Parser::InsertionModeBeforeHead(const Token& Token)
    {
        if (Token.Type == TokenType::Character && std::isspace(static_cast<unsigned char>(Token.Data[0]))) {
            return; // Ignore whitespace
        }
        if (Token.Type == TokenType::StartTag && Utils::ToLower(Token.Data) == "head")
        {
            InsertElement(Token);
            InsertionMode = InsertionMode::InHead;
        }
        else
        {
            // Implicitly create <head>
            HtmlParser::Token HeadToken;
            HeadToken.Type = TokenType::StartTag;
            HeadToken.Data = "head";
            InsertElement(HeadToken);
            InsertionMode = InsertionMode::InHead;
            InsertionModeInHead(Token);
        }
    }

    void Parser::InsertionModeInHead(const Token& Token)
    {
        if (Token.Type == TokenType::Character && std::isspace(static_cast<unsigned char>(Token.Data[0]))) {
            return; // Ignore whitespace
        }
        if (Token.Type == TokenType::EndTag && Utils::ToLower(Token.Data) == "head")
        {
            OpenElements.pop_back();
            InsertionMode = InsertionMode::AfterHead;
        }
        else
        {
            // For simplicity, we'll close <head> here
            OpenElements.pop_back();
            InsertionMode = InsertionMode::AfterHead;
            InsertionModeAfterHead(Token);
        }
    }

    void Parser::InsertionModeAfterHead(const Token& Token)
    {
        if (Token.Type == TokenType::Character && std::isspace(static_cast<unsigned char>(Token.Data[0]))) {
            return; // Ignore whitespace
        }
        if (Token.Type == TokenType::StartTag && Utils::ToLower(Token.Data) == "body")
        {
            InsertElement(Token);
            InsertionMode = InsertionMode::InBody;
        }
        else
        {
            // Implicitly create <body>
            HtmlParser::Token BodyToken;
            BodyToken.Type = TokenType::StartTag;
            BodyToken.Data = "body";
            InsertElement(BodyToken);
            InsertionMode = InsertionMode::InBody;
            InsertionModeInBody(Token);
        }
    }

    void Parser::InsertionModeInBody(const Token& Token)
    {
        if (Token.Type == TokenType::Character)
        {
            InsertCharacter(Token);
        }
        else if (Token.Type == TokenType::StartTag)
        {
            InsertElement(Token);
        }
        else if (Token.Type == TokenType::EndTag)
        {
            CloseElement(Token);
        }
        else
        {
            // Handle other token types as needed
        }
    }
} // namespace HtmlParser
