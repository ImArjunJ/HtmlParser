#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>
#include <iostream>

int main()
{
    // HTML sample with nested elements and classes
    std::string Html = R"(
        <div class="container">
            <h1>Title</h1>
            <p class="intro">Introduction paragraph.</p>
            <p class="content">Main content paragraph.</p>
            <p class="intro highlight">Another introduction.</p>
        </div>
    )";

    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);
    HtmlParser::Query Query(DOM.Root());

    // Select all paragraphs with class "intro"
    auto IntroParas = Query.Select("p.intro");

    std::cout << "Found " << IntroParas.size() << " paragraphs with class 'intro':\n";

    for (const auto& Paragraph : IntroParas)
    {
        std::cout << "- " << Paragraph->GetTextContent() << "\n";
    }

    // Select paragraph with classes "intro" and "highlight"
    auto HighlightedIntro = Query.SelectFirst("p.intro.highlight");

    if (HighlightedIntro)
    {
        std::cout << "Highlighted intro paragraph: " << HighlightedIntro->GetTextContent() << "\n";
    }

    return 0;
}