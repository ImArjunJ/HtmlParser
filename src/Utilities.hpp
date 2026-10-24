#pragma once
#include <HtmlParser/Diagnostics.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <cctype>
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

    inline std::string ToLower(const std::string& Input)
    {
        std::string Result = Input;
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

    inline ParseError MakeError(std::string_view Input, std::size_t Offset, std::string Message)
    {
        ParseError Result{Offset, 1, 1, std::move(Message)};
        for (std::size_t Index = 0; Index < std::min(Offset, Input.size()); ++Index)
        {
            if (Input[Index] == '\r' || (Input[Index] == '\n' && (Index == 0 || Input[Index - 1] != '\r')))
            {
                ++Result.Line;
                Result.Column = 1;
            }
            else if (Input[Index] != '\n')
                ++Result.Column;
        }
        return Result;
    }

    inline std::string NormalizeText(std::string_view Input)
    {
        std::string Result;
        Result.reserve(Input.size());
        for (std::size_t Index = 0; Index < Input.size(); ++Index)
        {
            if (Input[Index] == '\r')
            {
                Result += '\n';
                if (Index + 1 < Input.size() && Input[Index + 1] == '\n')
                    ++Index;
            }
            else if (Input[Index] == '\0')
                AppendCodepointUtf8(Result, 0xFFFD);
            else
                Result += Input[Index];
        }
        return Result;
    }

    inline std::string DecodeHtml(std::string_view Input, bool Attribute = false)
    {
        struct Entity { std::string_view Name; std::string_view Value; };
        static constexpr Entity Entities[] = {
#include "Entities.inc"
        };
        static constexpr unsigned int Windows1252[] = {
            0x20AC, 0x81, 0x201A, 0x192, 0x201E, 0x2026, 0x2020, 0x2021,
            0x2C6, 0x2030, 0x160, 0x2039, 0x152, 0x8D, 0x17D, 0x8F,
            0x90, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
            0x2DC, 0x2122, 0x161, 0x203A, 0x153, 0x9D, 0x17E, 0x178
        };
        auto IsAlphaNumeric = [](char Value) {
            return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z') || (Value >= '0' && Value <= '9');
        };
        std::string Result;
        Result.reserve(Input.size());
        for (std::size_t Position = 0; Position < Input.size();)
        {
            if (Input[Position] != '&')
            {
                Result += Input[Position++];
                continue;
            }
            const auto Start = Position + 1;
            if (Start < Input.size() && Input[Start] == '#')
            {
                auto End = Start + 1;
                const bool Hex = End < Input.size() && (Input[End] == 'x' || Input[End] == 'X');
                if (Hex)
                    ++End;
                const auto Digits = End;
                unsigned int Codepoint = 0;
                while (End < Input.size())
                {
                    const auto Value = Input[End];
                    const int Digit = Value >= '0' && Value <= '9' ? Value - '0' :
                        Hex && Lower(Value) >= 'a' && Lower(Value) <= 'f' ? Lower(Value) - 'a' + 10 : -1;
                    if (Digit < 0)
                        break;
                    Codepoint = Codepoint > 0x10FFFF ? 0x110000 : Codepoint * (Hex ? 16 : 10) + static_cast<unsigned int>(Digit);
                    ++End;
                }
                if (End != Digits)
                {
                    if (End < Input.size() && Input[End] == ';')
                        ++End;
                    if (Codepoint == 0 || Codepoint > 0x10FFFF || (Codepoint >= 0xD800 && Codepoint <= 0xDFFF))
                        Codepoint = 0xFFFD;
                    else if (Codepoint >= 0x80 && Codepoint <= 0x9F)
                        Codepoint = Windows1252[Codepoint - 0x80];
                    AppendCodepointUtf8(Result, Codepoint);
                    Position = End;
                    continue;
                }
            }
            else
            {
                auto End = Start;
                // The longest HTML entity name is 32 bytes including its semicolon.
                while (End < Input.size() && End - Start < 32 && IsAlphaNumeric(Input[End]))
                    ++End;
                if (End < Input.size() && Input[End] == ';')
                    ++End;
                for (; End > Start; --End)
                {
                    const auto Name = Input.substr(Start, End - Start);
                    const auto Found = std::lower_bound(std::begin(Entities), std::end(Entities), Name,
                        [](const auto& Entry, auto Key) { return Entry.Name < Key; });
                    if (Found == std::end(Entities) || Found->Name != Name)
                        continue;
                    if (Attribute && Name.back() != ';' && End < Input.size() && (IsAlphaNumeric(Input[End]) || Input[End] == '='))
                        break;
                    Result += Found->Value;
                    Position = End;
                    break;
                }
                if (Position == End)
                    continue;
            }
            Result += Input[Position++];
        }
        return Result;
    }
} // namespace HtmlParser::Utils
