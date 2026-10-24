#include "Utilities.hpp"

namespace HtmlParser::Utils
{
    ParseError MakeError(std::string_view Input, std::size_t Offset, std::string Message)
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

    std::string NormalizeText(std::string_view Input)
    {
        if (Input.find('\r') == std::string_view::npos && Input.find('\0') == std::string_view::npos)
            return std::string(Input);
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

    std::string DecodeHtml(std::string_view Input, bool Attribute)
    {
        if (Input.find('&') == std::string_view::npos)
            return std::string(Input);
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
