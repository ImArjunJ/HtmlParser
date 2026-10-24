#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "DOM.hpp"
#include "Tokenizer.hpp"

namespace HtmlParser
{
    struct ParseOptions
    {
        bool Strict = false;
        std::size_t MaxDepth = 4096;
        std::size_t MaxNodes = 1000000;
        std::size_t MaxErrors = 100;
    };

    struct ParseResult
    {
        DOM Document;
        std::vector<ParseError> Errors;
    };

    class Parser
    {
    public:
        Parser() = default;
        explicit Parser(ParseOptions Options) : m_Options(Options) {}

        DOM Parse(std::string_view Input) const;
        DOM ParseFragment(std::string_view Input, std::string_view ContextTag = "body") const;
        ParseResult ParseWithDiagnostics(std::string_view Input) const;
        ParseResult ParseFragmentWithDiagnostics(std::string_view Input, std::string_view ContextTag = "body") const;

        void SetStrict(bool Strict) noexcept { m_Options.Strict = Strict; }
        const ParseOptions& Options() const noexcept { return m_Options; }

    private:
        ParseOptions m_Options;
    };
} // namespace HtmlParser
