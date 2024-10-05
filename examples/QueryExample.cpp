#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>
#include <iostream>

int main()
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p><p>Another Paragraph</p></body></html>");

    HtmlParser::Query Query;
    auto Elements = Query.FindAll(DOM, "p");

    for (const auto& Node : Elements)
    {
        std::cout << "Found <p> tag with text: " << Node->Text << std::endl;
    }
    return 0;
}
