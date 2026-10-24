#include <HtmlParser/HtmlParser.hpp>
#include <iostream>

int main()
{
    const auto Fragment = HtmlParser::ParseFragment("<p>One</p><p>Two</p>");
    for (const auto& Paragraph : Fragment.Select("p"))
        std::cout << Paragraph->GetTextContent() << '\n';
    std::cout << Fragment.ToHtml() << '\n';

    const auto Table = HtmlParser::ParseFragment("<tr><td>Cell", "table");
    std::cout << Table.ToHtml() << '\n';
}
