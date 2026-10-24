#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace HtmlParser
{
    struct ParseError
    {
        std::size_t Offset = 0;
        std::size_t Line = 1;
        std::size_t Column = 1;
        std::string Message;
    };

    class ParseException : public std::runtime_error
    {
    public:
        explicit ParseException(ParseError Error)
            : std::runtime_error("Parse error at " + std::to_string(Error.Line) + ":" +
                std::to_string(Error.Column) + ": " + Error.Message), m_Error(std::move(Error)) {}

        const ParseError& Error() const noexcept { return m_Error; }

    private:
        ParseError m_Error;
    };
}
