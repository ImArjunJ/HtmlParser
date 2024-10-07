#include <HtmlParser/Tokenizer.hpp>

namespace HtmlParser
{
    Tokenizer::Tokenizer(const std::string& InputStr) : Input(InputStr), Position(0), CurrentState(State::Data)
    {
    }

    void Tokenizer::Tokenize()
    {
        while (Position < Input.size())
        {
            char c = Input[Position++];
            switch (CurrentState)
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
        return Tokens;
    }

    void Tokenizer::EmitToken(const Token& token)
    {
        Tokens.push_back(token);
    }

    void Tokenizer::ReconsumeChar()
    {
        --Position;
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
            CurrentState = State::TagOpen;
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
            CurrentState = State::EndTagOpen;
        }
        else if (IsAlpha(c))
        {
            CurrentToken = Token();
            CurrentToken.Type = TokenType::StartTag;
            CurrentToken.Data = c;
            CurrentState = State::TagName;
        }
        else
        {
            // Parse error
            CurrentState = State::Data;
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
            CurrentState = State::BeforeAttributeName;
        }
        else if (c == '/')
        {
            CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '>')
        {
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else
        {
            CurrentToken.Data += c;
        }
    }

    void Tokenizer::HandleEndTagOpenState(char c)
    {
        if (IsAlpha(c))
        {
            CurrentToken = Token();
            CurrentToken.Type = TokenType::EndTag;
            CurrentToken.Data = c;
            CurrentState = State::TagName;
        }
        else
        {
            // Parse error
            CurrentState = State::Data;
        }
    }

    void Tokenizer::HandleSelfClosingStartTagState(char c)
    {
        if (c == '>')
        {
            CurrentToken.SelfClosing = true;
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else
        {
            // Parse error
            CurrentState = State::BeforeAttributeName;
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
            CurrentState = State::AfterAttributeName;
            ReconsumeChar();
        }
        else
        {
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            CurrentState = State::AttributeName;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleAttributeNameState(char c)
    {
        if (IsWhitespace(c) || c == '/' || c == '>')
        {
            CurrentState = State::AfterAttributeName;
            ReconsumeChar();
        }
        else if (c == '=')
        {
            CurrentState = State::BeforeAttributeValue;
        }
        else
        {
            CurrentAttributeName += c;
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
            CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '=')
        {
            CurrentState = State::BeforeAttributeValue;
        }
        else if (c == '>')
        {
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else
        {
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            CurrentState = State::AttributeName;
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
            CurrentState = State::AttributeValueDoubleQuoted;
        }
        else if (c == '\'')
        {
            CurrentState = State::AttributeValueSingleQuoted;
        }
        else if (c == '>')
        {
            // Parse error
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else
        {
            CurrentState = State::AttributeValueUnquoted;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleAttributeValueDoubleQuotedState(char c)
    {
        if (c == '"')
        {
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            CurrentState = State::AfterAttributeValueQuoted;
        }
        else
        {
            CurrentAttributeValue += c;
        }
    }

    void Tokenizer::HandleAttributeValueSingleQuotedState(char c)
    {
        if (c == '\'')
        {
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            CurrentState = State::AfterAttributeValueQuoted;
        }
        else
        {
            CurrentAttributeValue += c;
        }
    }

    void Tokenizer::HandleAttributeValueUnquotedState(char c)
    {
        if (IsWhitespace(c))
        {
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            CurrentState = State::AfterAttributeValueUnquoted;
        }
        else if (c == '>')
        {
            CurrentToken.Attributes[CurrentAttributeName] = CurrentAttributeValue;
            CurrentAttributeName.clear();
            CurrentAttributeValue.clear();
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else if (c == '&')
        {
            CurrentAttributeValue += c;
        }
        else if (c == '\0')
        {
            // Parse error
        }
        else if (c == '"' || c == '\'' || c == '<' || c == '=' || c == '`')
        {
            // Parse error
            CurrentAttributeValue += c;
        }
        else
        {
            CurrentAttributeValue += c;
        }
    }

    void Tokenizer::HandleAfterAttributeValueQuotedState(char c)
    {
        if (IsWhitespace(c))
        {
            CurrentState = State::BeforeAttributeName;
        }
        else if (c == '/')
        {
            CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '>')
        {
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else
        {
            // Parse error
            CurrentState = State::BeforeAttributeName;
            ReconsumeChar();
        }
    }

    void Tokenizer::HandleAfterAttributeValueUnquotedState(char c)
    {
        if (IsWhitespace(c))
        {
            CurrentState = State::BeforeAttributeName;
        }
        else if (c == '/')
        {
            CurrentState = State::SelfClosingStartTag;
        }
        else if (c == '>')
        {
            EmitToken(CurrentToken);
            CurrentState = State::Data;
        }
        else
        {
            // Parse error
            CurrentState = State::BeforeAttributeName;
            ReconsumeChar();
        }
    }
} // namespace HtmlParser