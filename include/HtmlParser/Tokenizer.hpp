#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace HtmlParser
{
    enum class TokenType
    {
        DOCTYPE,
        StartTag,
        EndTag,
        Comment,
        Character,
        EOFToken
    };

    struct Token
    {
        TokenType Type;
        std::string Data;
        std::unordered_map<std::string, std::string> Attributes;
        bool SelfClosing = false;
    };

    class Tokenizer
    {
    public:
        Tokenizer(const std::string& InputStr);
        void Tokenize();
        const std::vector<Token>& GetTokens() const;

    private:
        enum class State
        {
            Data,
            TagOpen,
            TagName,
            EndTagOpen,
            SelfClosingStartTag,
            BeforeAttributeName,
            AttributeName,
            AfterAttributeName,
            BeforeAttributeValue,
            AttributeValueDoubleQuoted,
            AttributeValueSingleQuoted,
            AttributeValueUnquoted,
            AfterAttributeValueQuoted,
            AfterAttributeValueUnquoted,
        };

        void EmitToken(const Token& Token);
        void ReconsumeChar();

        bool IsWhitespace(char c) const;
        bool IsAlpha(char c) const;

        // State handling functions
        void HandleDataState(char c);
        void HandleTagOpenState(char c);
        void HandleTagNameState(char c);
        void HandleEndTagOpenState(char c);
        void HandleSelfClosingStartTagState(char c);
        void HandleBeforeAttributeNameState(char c);
        void HandleAttributeNameState(char c);
        void HandleAfterAttributeNameState(char c);
        void HandleBeforeAttributeValueState(char c);
        void HandleAttributeValueDoubleQuotedState(char c);
        void HandleAttributeValueSingleQuotedState(char c);
        void HandleAttributeValueUnquotedState(char c);
        void HandleAfterAttributeValueQuotedState(char c);
        void HandleAfterAttributeValueUnquotedState(char c);

        std::string m_Input;
        size_t m_Position;
        State m_CurrentState;

        Token m_CurrentToken;
        std::string m_CurrentAttributeName;
        std::string m_CurrentAttributeValue;

        std::vector<Token> m_Tokens;
    };
} // namespace HtmlParser