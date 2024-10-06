#pragma once
#include <algorithm>
#include <string>
#include <vector>

namespace HtmlParser::Utils
{
    inline std::string Trim(const std::string& Input)
    {
        const size_t Start = Input.find_first_not_of(" \t\n\r");
        const size_t End = Input.find_last_not_of(" \t\n\r");
        return (Start == std::string::npos) ? "" : Input.substr(Start, End - Start + 1);
    }

    inline bool IsSelfClosingTag(const std::string& Tag)
    {
        const std::vector<std::string> SelfClosingTags = {"area", "base", "br", "col", "embed", "hr", "img", "input", "keygen", "link", "meta", "param", "source", "track", "wbr"};
        return std::find(SelfClosingTags.begin(), SelfClosingTags.end(), Tag) != SelfClosingTags.end();
    }
} // namespace HtmlParser::Utils