# HtmlParser

A cross-platform C++ HTML parser library inspired by BeautifulSoup, featuring DOM manipulation and querying support.

## Table of Contents

- [Installation](#installation)
- [Basic Usage](#basic-usage)
  - [Parsing HTML](#parsing-html)
  - [Querying Nodes](#querying-nodes)
- [Advanced Examples](#advanced-examples)
  - [Handling Nested Elements](#handling-nested-elements)
  - [Using Query Selectors](#using-query-selectors)
- [Contributing](#contributing)

## Installation

The best way to install HtmlParser is via CMake, using `FetchContent`

```cmake
include(FetchContent)
FetchContent_Declare(
        HtmlParser
        GIT_REPOSITORY https://github.com/ImArjunJ/htmlparser.git
        GIT_TAG master
)

option(HTMLPARSER_BUILD_TESTS "Build tests" OFF)
option(HTMLPARSER_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(HTMLPARSER_BUILD_EXAMPLES "Build examples" OFF)
FetchContent_MakeAvailable(HtmlParser)

...

target_link_libraries(TargetProject PRIVATE HtmlParser ...)
```

## Basic Usage

### Parsing HTML

```c++
#include <HtmlParser/Parser.hpp>

int main()
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");
    return 0;
}
```

### Querying Nodes

```c++
#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>
#include <iostream>

int main()
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");
    HtmlParser::Query Query(DOM.Root());

    auto Paragraphs = Query.SelectFirst("p");
    if (Paragraphs)
    {
        std::cout << "Found paragraph: " << Paragraphs->GetTextContent() << std::endl;
    }

    return 0;
}
```

**Output:**

```
Found paragraph: Hello World
```

## Advanced Examples

### Handling Nested Elements

```c++
#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML with nested elements
    std::string Html = R"(
        <div>
            <h1>Welcome</h1>
            <p>This is a <strong>sample</strong> paragraph with <em>nested</em> elements.</p>
        </div>
    )";

    HtmlParser::Parser Parser;
    const HtmlParser::DOM DOM = Parser.Parse(Html);

    const auto Paragraph = DOM.GetElementsByTagName("p").front();

    if (Paragraph)
    {
        // Extract the text content, including text from nested elements
        const std::string Text = Paragraph->GetTextContent();
        std::cout << "Paragraph text: " << Text << "\n";
    }
    else
    {
        std::cout << "Paragraph not found.\n";
    }

    return 0;
}
```

**Output:**

```
Paragraph text: This is a sample paragraph with nested elements.
```

### Using Query Selectors

```c++
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
```

**Output:**

```
Found 2 paragraphs with class 'intro':
- Introduction paragraph.
- Another introduction.
Highlighted intro paragraph: Another introduction.
```

## Contributing

Contributions are welcome! Please open issues or submit pull requests to improve **HtmlParser**.
