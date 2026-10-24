#pragma once
#include <HtmlParser/Diagnostics.hpp>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace HtmlParser::Utils
{
    constexpr bool IsWhitespace(char Value)
    {
        return Value == ' ' || Value == '\t' || Value == '\n' || Value == '\r' || Value == '\f';
    }

    constexpr char Lower(char Value)
    {
        return Value >= 'A' && Value <= 'Z' ? static_cast<char>(Value + ('a' - 'A')) : Value;
    }

    inline bool EqualsIgnoreCase(std::string_view Left, std::string_view Right)
    {
        return Left.size() == Right.size() && std::equal(Left.begin(), Left.end(), Right.begin(),
            [](char A, char B) { return Lower(A) == Lower(B); });
    }

    inline std::vector<std::string_view> Words(std::string_view Input)
    {
        std::vector<std::string_view> Result;
        std::size_t Position = 0;
        while (Position < Input.size())
        {
            while (Position < Input.size() && IsWhitespace(Input[Position]))
                ++Position;
            const auto Start = Position;
            while (Position < Input.size() && !IsWhitespace(Input[Position]))
                ++Position;
            if (Position != Start)
                Result.push_back(Input.substr(Start, Position - Start));
        }
        return Result;
    }

    inline bool ContainsWord(std::string_view Input, std::string_view Word)
    {
        if (Word.empty() || std::any_of(Word.begin(), Word.end(), IsWhitespace))
            return false;
        std::size_t Position = 0;
        while ((Position = Input.find(Word, Position)) != std::string_view::npos)
        {
            const auto End = Position + Word.size();
            if ((Position == 0 || IsWhitespace(Input[Position - 1])) && (End == Input.size() || IsWhitespace(Input[End])))
                return true;
            Position = End;
        }
        return false;
    }

    inline bool IsVoidElement(std::string_view Tag)
    {
        constexpr std::string_view Tags[] = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "param", "source", "track", "wbr"};
        return std::any_of(std::begin(Tags), std::end(Tags), [&](auto Value) { return EqualsIgnoreCase(Tag, Value); });
    }

    inline bool IsRawTextElement(std::string_view Tag)
    {
        constexpr std::string_view Tags[] = {"script", "style", "xmp", "iframe", "noembed", "noframes", "plaintext"};
        return std::any_of(std::begin(Tags), std::end(Tags), [&](auto Value) { return EqualsIgnoreCase(Tag, Value); });
    }

    inline std::string ToLower(std::string_view Input)
    {
        std::string Result(Input);
        std::transform(Result.begin(), Result.end(), Result.begin(), Lower);
        return Result;
    }

    inline std::string Trim(const std::string& Input)
    {
        const std::string Whitespace = " \t\n\r\f";
        size_t Start = Input.find_first_not_of(Whitespace);
        if (Start == std::string::npos)
            return "";
        size_t End = Input.find_last_not_of(Whitespace);
        return Input.substr(Start, End - Start + 1);
    }

    inline void AppendEscaped(std::string& Escaped, std::string_view Input)
    {
        for (char c : Input)
        {
            switch (c)
            {
            case '&':
                Escaped.append("&amp;");
                break;
            case '<':
                Escaped.append("&lt;");
                break;
            case '>':
                Escaped.append("&gt;");
                break;
            case '"':
                Escaped.append("&quot;");
                break;
            case '\'':
                Escaped.append("&#39;");
                break;
            default:
                Escaped.push_back(c);
                break;
            }
        }
    }

    inline std::string EscapeHtml(const std::string& Input)
    {
        std::string Escaped;
        AppendEscaped(Escaped, Input);
        return Escaped;
    }

    inline void AppendCodepointUtf8(std::string& Output, unsigned int Codepoint)
    {
        if (Codepoint <= 0x7F)
        {
            Output.push_back(static_cast<char>(Codepoint));
        }
        else if (Codepoint <= 0x7FF)
        {
            Output.push_back(static_cast<char>(0xC0 | (Codepoint >> 6)));
            Output.push_back(static_cast<char>(0x80 | (Codepoint & 0x3F)));
        }
        else if (Codepoint <= 0xFFFF)
        {
            Output.push_back(static_cast<char>(0xE0 | (Codepoint >> 12)));
            Output.push_back(static_cast<char>(0x80 | ((Codepoint >> 6) & 0x3F)));
            Output.push_back(static_cast<char>(0x80 | (Codepoint & 0x3F)));
        }
        else if (Codepoint <= 0x10FFFF)
        {
            Output.push_back(static_cast<char>(0xF0 | (Codepoint >> 18)));
            Output.push_back(static_cast<char>(0x80 | ((Codepoint >> 12) & 0x3F)));
            Output.push_back(static_cast<char>(0x80 | ((Codepoint >> 6) & 0x3F)));
            Output.push_back(static_cast<char>(0x80 | (Codepoint & 0x3F)));
        }
    }

    ParseError MakeError(std::string_view Input, std::size_t Offset, std::string Message);
    std::string NormalizeText(std::string_view Input);
    std::string DecodeHtml(std::string_view Input, bool Attribute = false);
} // namespace HtmlParser::Utils
