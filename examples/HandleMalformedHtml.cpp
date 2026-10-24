#include <HtmlParser/HtmlParser.hpp>
#include <iostream>

int main()
{
    HtmlParser::Parser Parser;
    const auto Result = Parser.ParseWithDiagnostics("<div>\n</unexpected></div>");
    for (const auto& Error : Result.Errors)
        std::cout << Error.Line << ':' << Error.Column << ": " << Error.Message << '\n';

    Parser.SetStrict(true);
    try
    {
        Parser.Parse("<p>Unclosed paragraph");
    }
    catch (const HtmlParser::ParseException& Error)
    {
        std::cout << Error.what() << '\n';
    }
}
