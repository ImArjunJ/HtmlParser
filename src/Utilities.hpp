#pragma once
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

namespace HtmlParser::Utils
{
    inline std::string ToLower(const std::string& Input)
    {
        std::string Result = Input;
        std::transform(Result.begin(), Result.end(), Result.begin(), [](unsigned char c) { return std::tolower(c); });
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

    inline std::string EscapeHtml(const std::string& Input)
    {
        std::string Escaped;
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

    inline std::string DecodeHtml(const std::string& Input)
    {
        std::string Decoded;
        for (size_t i = 0; i < Input.size(); ++i)
        {
            if (Input[i] != '&')
            {
                Decoded.push_back(Input[i]);
                continue;
            }

            const size_t Semicolon = Input.find(';', i + 1);
            if (Semicolon == std::string::npos)
            {
                Decoded.push_back(Input[i]);
                continue;
            }

            const std::string Entity = Input.substr(i + 1, Semicolon - i - 1);
            if (Entity == "amp")
            {
                Decoded.push_back('&');
            }
            else if (Entity == "lt")
            {
                Decoded.push_back('<');
            }
            else if (Entity == "gt")
            {
                Decoded.push_back('>');
            }
            else if (Entity == "quot")
            {
                Decoded.push_back('"');
            }
            else if (Entity == "apos")
            {
                Decoded.push_back('\'');
            }
            else if (Entity == "nbsp")
            {
                AppendCodepointUtf8(Decoded, 0xA0);
            }
            else if (!Entity.empty() && Entity[0] == '#')
            {
                char* End = nullptr;
                const bool IsHex = Entity.size() > 2 && (Entity[1] == 'x' || Entity[1] == 'X');
                const char* Start = Entity.c_str() + (IsHex ? 2 : 1);
                unsigned long Codepoint = std::strtoul(Start, &End, IsHex ? 16 : 10);
                if (End != Start && *End == '\0' && Codepoint <= 0x10FFFF)
                {
                    AppendCodepointUtf8(Decoded, static_cast<unsigned int>(Codepoint));
                }
                else
                {
                    Decoded.append(Input, i, Semicolon - i + 1);
                }
            }
            else
            {
                Decoded.append(Input, i, Semicolon - i + 1);
            }

            i = Semicolon;
        }
        return Decoded;
    }
} // namespace HtmlParser::Utils
