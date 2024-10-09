#include <HtmlParser/Tokenizer.hpp>

namespace HtmlParser
{
    Tokenizer::Tokenizer(const std::string& InputStr) : m_Input(InputStr), m_Position(0), m_CurrentState(State::Data)
    {
    }

    void Tokenizer::Tokenize()
    {
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
    }

    const std::vector<Token>& Tokenizer::GetTokens() const
    {
        return m_Tokens;
    }

    void Tokenizer::EmitToken(const Token& Token)
    {
        m_Tokens.push_back(Token);
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

    // Implement state handling functions
    void Tokenizer::HandleDataState(char c)
    {
        if (c == '<')
        {
            m_CurrentState = State::TagOpen;
        }
        else
        {
            Token Token;
            Token.Type = TokenType::Character;
            Token.Data = c;
            EmitToken(Token);
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
        else
        {
            // Parse error
            m_CurrentState = State::Data;
            Token Token;
            Token.Type = TokenType::Character;
            Token.Data = '<';
            EmitToken(Token);
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
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
            EmitToken(m_CurrentToken);
            m_CurrentState = State::Data;
        }
        else
        {
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
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
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
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
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
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
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
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
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
            m_CurrentState = State::AfterAttributeValueUnquoted;
        }
        else if (c == '>')
        {
            m_CurrentToken.Attributes[m_CurrentAttributeName] = m_CurrentAttributeValue;
            m_CurrentAttributeName.clear();
            m_CurrentAttributeValue.clear();
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