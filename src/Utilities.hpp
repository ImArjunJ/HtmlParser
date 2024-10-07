#pragma once
#include <algorithm>
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
} // namespace HtmlParser::Utils