#include <HtmlParser/Tokenizer.hpp>

#include <cctype>

#include "Utilities.hpp"

namespace HtmlParser
{
    Tokenizer::Tokenizer(const std::string& InputStr) : m_Input(InputStr), m_Position(0), m_CurrentState(State::Data)
    {
    }

    void Tokenizer::Tokenize()
    {
        m_Position = 0;
        m_CurrentState = State::Data;
        m_CurrentText.clear();
        m_CurrentAttributeName.clear();
        m_CurrentAttributeValue.clear();
        m_Tokens.clear();

        while (m_Position < m_Input.size())
        {
            char c = m_Input[m_Position++];
            switch (m_CurrentState)
            {
            case State::Data:
                HandleDataState(c);
                break;
            case State::TagOpen:
                HandleTagOpenState(c);
                break;
            case State::TagName:
                HandleTagNameState(c);
                break;
            case State::EndTagOpen:
                HandleEndTagOpenState(c);
                break;
            case State::SelfClosingStartTag:
                HandleSelfClosingStartTagState(c);
                break;
            case State::BeforeAttributeName:
                HandleBeforeAttributeNameState(c);
                break;
            case State::AttributeName:
                HandleAttributeNameState(c);
                break;
            case State::AfterAttributeName:
                HandleAfterAttributeNameState(c);
                break;
            case State::BeforeAttributeValue:
                HandleBeforeAttributeValueState(c);
                break;
            case State::AttributeValueDoubleQuoted:
                HandleAttributeValueDoubleQuotedState(c);
                break;
            case State::AttributeValueSingleQuoted:
                HandleAttributeValueSingleQuotedState(c);
                break;
            case State::AttributeValueUnquoted:
                HandleAttributeValueUnquotedState(c);
                break;
            case State::AfterAttributeValueQuoted:
                HandleAfterAttributeValueQuotedState(c);
                break;
            case State::AfterAttributeValueUnquoted:
                HandleAfterAttributeValueUnquotedState(c);
                break;
            }
        }
        EmitCurrentText();
    }

    const std::vector<Token>& Tokenizer::GetTokens() const
    {
        return m_Tokens;
    }

    void Tokenizer::EmitToken(const Token& Token)
    {
        m_Tokens.push_back(Token);
    }

    void Tokenizer::EmitCurrentText()
    {
        if (m_CurrentText.empty())
        {
            return;
        }

        Token Token;
        Token.Type = TokenType::Character;
        Token.Data = Utils::DecodeHtml(m_CurrentText);
        EmitToken(Token);
        m_CurrentText.clear();
    }

    void Tokenizer::StoreCurrentAttribute()
    {
        if (!m_CurrentAttributeName.empty())
        {
            m_CurrentToken.Attributes[Utils::ToLower(m_CurrentAttributeName)] = Utils::DecodeHtml(m_CurrentAttributeValue);
        }
        m_CurrentAttributeName.clear();
        m_CurrentAttributeValue.clear();
    }

    void Tokenizer::ReconsumeChar()
    {
        --m_Position;
    }

    bool Tokenizer::IsWhitespace(char c) const
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
    }

    bool Tokenizer::IsAlpha(char c) const
    {
        return std::isalpha(static_cast<unsigned char>(c));
    }

    bool Tokenizer::StartsWithIgnoreCase(size_t Position, const std::string& Value) const
    {
        if (Position + Value.size() > m_Input.size())
        {
            return false;
        }

        for (size_t i = 0; i < Value.size(); ++i)
        {
            const auto InputChar = static_cast<unsigned char>(m_Input[Position + i]);
            const auto ValueChar = static_cast<unsigned char>(Value[i]);
            if (std::tolower(InputChar) != std::tolower(ValueChar))
            {
                return false;
            }
        }
        return true;
    }

    // Implement state handling functions
    void Tokenizer::HandleDataState(char c)
    {
        if (c == '<')
        {
            EmitCurrentText();
            m_CurrentState = State::TagOpen;
        }
        else
        {
            m_CurrentText += c;
        }
    }

    void Tokenizer::HandleTagOpenState(char c)
    {
        if (c == '/')
        {
            m_CurrentState = State::EndTagOpen;
        }
        else if (IsAlpha(c))
        {
            m_CurrentToken = Token();
            m_CurrentToken.Type = TokenType::StartTag;
            m_CurrentToken.Data = c;
            m_CurrentState = State::TagName;
        }
        else if (c == '!')
        {
            if (m_Position + 2 <= m_Input.size() && m_Input.compare(m_Position, 2, "--") == 0)
            {
                const size_t CommentStart = m_Position + 2;
                const size_t CommentEnd = m_Input.find("-->", CommentStart);

                Token Token;
                Token.Type = TokenType::Comment;
                if (CommentEnd == std::string::npos)
                {
                    Token.Data = m_Input.substr(CommentStart);
                    m_Position = m_Input.size();
                }
                else
                {
                    Token.Data = m_Input.substr(CommentStart, CommentEnd - CommentStart);
                    m_Position = CommentEnd + 3;
                }
                EmitToken(Token);
            }
            else if (StartsWithIgnoreCase(m_Position, "DOCTYPE"))
            {
                const size_t DoctypeStart = m_Position + 7;
                const size_t DoctypeEnd = m_Input.find('>', DoctypeStart);

                Token Token;
                Token.Type = TokenType::DOCTYPE;
                if (DoctypeEnd == std::string::npos)
                {
                    Token.Data = Utils::Trim(m_Input.substr(DoctypeStart));
                    m_Position = m_Input.size();
                }
                else
                {
                    Token.Data = Utils::Trim(m_Input.substr(DoctypeStart, DoctypeEnd - DoctypeStart));
                    m_Position = DoctypeEnd + 1;
                }
                EmitToken(Token);
            }
            else
            {
                m_CurrentText += "<!";
            }
            m_CurrentState = State::Data;
        }
        else
        {
            // Parse error
            m_CurrentState = State::Data;
            m_CurrentText += '<';
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleTagNameState(char c)
    {
        if (IsWhitespace(c))
        {
            m_CurrentState = State::BeforeAttributeName;
        }
        else if (c == '/')
        {
            m_CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '>')
        {
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            m_CurrentToken.Data += c;
        }
    }

    void Tokenizer::HandleEndTagOpenState(char c)
    {
        if (IsAlpha(c))
        {
            m_CurrentToken = Token();
            m_CurrentToken.Type = TokenType::EndTag;
            m_CurrentToken.Data = c;
            m_CurrentState = State::TagName;
        }
        else
        {
            // Parse error
            m_CurrentState = State::Data;
        }
    }

    void Tokenizer::HandleSelfClosingStartTagState(char c)
    {
        if (c == '>')
        {
            m_CurrentToken.SelfClosing = true;
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            // Parse error
            m_CurrentState = State::BeforeAttributeName;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleBeforeAttributeNameState(char c)
    {
        if (IsWhitespace(c))
        {
            // Ignore
        }
        else if (c == '/' || c == '>')
        {
            m_CurrentState = State::AfterAttributeName;
            ReconsumeChar();
        }
        else
        {
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
            m_CurrentState = State::AttributeName;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleAttributeNameState(char c)
    {
        if (IsWhitespace(c) || c == '/' || c == '>')
        {
            m_CurrentState = State::AfterAttributeName;
            ReconsumeChar();
        }
        else if (c == '=')
        {
            m_CurrentState = State::BeforeAttributeValue;
        }
        else
        {
            m_CurrentAttributeName += c;
        }
    }

    void Tokenizer::HandleAfterAttributeNameState(char c)
    {
        if (IsWhitespace(c))
        {
            // Ignore
        }
        else if (c == '/')
        {
            m_CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '=')
        {
            m_CurrentState = State::BeforeAttributeValue;
        }
        else if (c == '>')
        {
            StoreCurrentAttribute();
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            StoreCurrentAttribute();
            m_CurrentState = State::AttributeName;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleBeforeAttributeValueState(char c)
    {
        if (IsWhitespace(c))
        {
            // Ignore
        }
        else if (c == '"')
        {
            m_CurrentState = State::AttributeValueDoubleQuoted;
        }
        else if (c == '\'')
        {
            m_CurrentState = State::AttributeValueSingleQuoted;
        }
        else if (c == '>')
        {
            // Parse error
            StoreCurrentAttribute();
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            m_CurrentState = State::AttributeValueUnquoted;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleAttributeValueDoubleQuotedState(char c)
    {
        if (c == '"')
        {
            StoreCurrentAttribute();
            m_CurrentState = State::AfterAttributeValueQuoted;
        }
        else
        {
            m_CurrentAttributeValue += c;
        }
    }

    void Tokenizer::HandleAttributeValueSingleQuotedState(char c)
    {
        if (c == '\'')
        {
            StoreCurrentAttribute();
            m_CurrentState = State::AfterAttributeValueQuoted;
        }
        else
        {
            m_CurrentAttributeValue += c;
        }
    }

    void Tokenizer::HandleAttributeValueUnquotedState(char c)
    {
        if (IsWhitespace(c))
        {
            StoreCurrentAttribute();
            m_CurrentState = State::AfterAttributeValueUnquoted;
        }
        else if (c == '>')
        {
            StoreCurrentAttribute();
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else if (c == '&')
        {
            m_CurrentAttributeValue += c;
        }
        else if (c == '\0')
        {
            // Parse error
        }
        else if (c == '"' || c == '\'' || c == '<' || c == '=' || c == '`')
        {
            // Parse error
            m_CurrentAttributeValue += c;
        }
        else
        {
            m_CurrentAttributeValue += c;
        }
    }

    void Tokenizer::HandleAfterAttributeValueQuotedState(char c)
    {
        if (IsWhitespace(c))
        {
            m_CurrentState = State::BeforeAttributeName;
        }
        else if (c == '/')
        {
            m_CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '>')
        {
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            // Parse error
            m_CurrentState = State::BeforeAttributeName;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleAfterAttributeValueUnquotedState(char c)
    {
        if (IsWhitespace(c))
        {
            m_CurrentState = State::BeforeAttributeName;
        }
        else if (c == '/')
        {
            m_CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '>')
        {
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            // Parse error
            m_CurrentState = State::BeforeAttributeName;
            ReconsumeChar();
        }
    }
} // namespace HtmlParser
