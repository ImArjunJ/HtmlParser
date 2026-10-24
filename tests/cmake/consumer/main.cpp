#include <HtmlParser/HtmlParser.hpp>

int main()
{
    auto Document = HtmlParser::ParseFragment("<p id=example>installed</p>");
    const auto Element = Document.SelectFirst("#example");
    return Element && Element->GetTextContent() == "installed" ? 0 : 1;
}
