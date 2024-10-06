#pragma once
#include <string>

#include "DOM.hpp"

namespace HtmlParser
{
    class Parser
    {
    public:
        // Parse HTML string and return the DOM
        DOM Parse(const std::string& RawHtml);

        // Set strict mode which is used to throw exceptions when the HTML is not well-formed
        void SetStrict(bool Strict);

    private:
        std::unordered_map<std::string, std::string> ParseAttributes(const std::string& Input);

    private:
        bool IsStrict = false;
    };
} // namespace HtmlParser
