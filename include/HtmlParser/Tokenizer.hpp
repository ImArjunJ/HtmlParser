#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Diagnostics.hpp"

namespace HtmlParser
{
    enum class TokenType { DOCTYPE, StartTag, EndTag, Comment, Character, EOFToken };

    struct Token
    {
        TokenType Type = TokenType::EOFToken;
        std::string Data;
        std::unordered_map<std::string, std::string> Attributes;
        bool SelfClosing = false;
        std::size_t Offset = 0;
        std::size_t EndOffset = 0;
    };

    class Tokenizer
    {
    public:
        // The public tokenizer owns its input; temporary strings are safe.
        explicit Tokenizer(std::string_view Input);
        Token NextToken();
        void Reset();
        void Tokenize();
        const std::vector<Token>& GetTokens() const noexcept;
        const std::vector<ParseError>& GetErrors() const noexcept;
        void SetTextMode(std::string_view Tag);

    private:
        friend class Parser;
        // Parser borrows input only for the duration of a synchronous parse.
        Tokenizer(std::string_view Input, bool Borrow);
        std::string_view Input() const noexcept;
        Token ReadTag();
        Token ReadDeclaration();
        Token ReadText();
        void Error(std::size_t Offset, std::string Message);
        bool StartsWith(std::size_t Offset, std::string_view Value) const;

        std::string m_Storage;
        std::string_view m_View;
        bool m_Borrowed = false;
        std::size_t m_Position = 0;
        std::string m_TextTag;
        bool m_DecodeText = true;
        std::vector<Token> m_Tokens;
        std::vector<ParseError> m_Errors;
    };
}
