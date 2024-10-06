# HTML Parser Library

A C++ HTML parser library, inspired by BeautifulSoup, with DOM manipulation and querying support.

## Basic Example Usage

# Parsing HTML

```c++
#include <HtmlParser/Parser.hpp>

HtmlParser::Parser Parser;
HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");
```

# Serializing to HTML

```c++
const std::string HtmlOutput = DOM.ToHtml();
std::cout << HtmlOutput << std::endl;  // Outputs: <html><body><p>Hello World</p></body></html>
```

# Querying Nodes

```c++
HtmlParser::Query Query;
auto Paragraph = Query.Find(DOM, "p");
if (!Paragraph.empty())
{
    std::cout << "Found paragraph: " << Paragraph[0]->text << std::endl;  // Outputs: Found paragraph: Hello World
}
```

# Retrieving Child Nodes

```c++
auto Body = DOM.Root()->Find("body");
if (Body)
{
    auto Children = Body->GetChildren();
    for (const auto& Child : Children)
    {
        std::cout << "Child tag: " << Child->Tag << std::endl;  // Outputs: Child tag: p
    }
}
```

# Strict Mode

To enable strict mode during parsing, use:

```c++
Parser.SetStrict(true); // Enables strict mode
```

# Contributing

Contributions are welcome! Please open issues or submit some pull requests to improve HtmlParser.
