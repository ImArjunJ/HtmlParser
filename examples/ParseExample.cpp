#include <HtmlParser/Parser.hpp>
#include <iostream>


int main()
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");

    std::cout << "Parsed HTML: " << DOM.ToHtml() << std::endl;
    return 0;
}
