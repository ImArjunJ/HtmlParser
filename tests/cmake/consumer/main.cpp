#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>

int main()
{
    auto Document = HtmlParser::Parser().ParseFragment("<p id=example>installed</p>");
    const auto Element = Document.SelectFirst("#example");
    return Element && Element->GetTextContent() == "installed" ? 0 : 1;
}
