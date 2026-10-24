#include <HtmlParser/HtmlParser.hpp>
#include <iostream>

int main()
{
    const auto Document = HtmlParser::Parse("<ul><li>One<li class=skip>Two<li>Three</ul>");
    const HtmlParser::Selector Pattern("ul > li:not(.skip):nth-of-type(odd)");
    if (!Pattern.IsValid())
    {
        std::cerr << Pattern.Error() << '\n';
        return 1;
    }
    for (const auto& Item : HtmlParser::Query(Document.Root()).Select(Pattern))
        std::cout << Item->GetTextContent() << '\n';
}
