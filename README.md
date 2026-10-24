# HtmlParser

A C++20 HTML parser with a mutable DOM, CSS queries, fragment parsing, and useful error reporting. The library has no runtime dependencies and builds on Linux, macOS, and Windows.

```cpp
#include <HtmlParser/HtmlParser.hpp>
#include <iostream>

int main()
{
    auto Document = HtmlParser::Parse(R"(
        <article>
            <h1>A &amp; B</h1>
            <a class="download" href="/file.zip">Download</a>
        </article>
    )");

    if (auto Heading = Document.SelectFirst("article > h1"))
        std::cout << Heading->GetTextContent() << '\n'; // A & B

    for (const auto& Link : Document.Select("a.download[href$='.zip']"))
        std::cout << Link->GetAttribute("href") << '\n';
}
```

## Installation

With CMake `FetchContent`:

```cmake
include(FetchContent)
FetchContent_Declare(HtmlParser
    GIT_REPOSITORY https://github.com/ImArjunJ/HtmlParser.git
    GIT_TAG master # Pin a commit for reproducible builds.
)
FetchContent_MakeAvailable(HtmlParser)
target_link_libraries(your_target PRIVATE HtmlParser::HtmlParser)
```

Tests and examples are disabled by default when embedded in another project. The target supplies the C++20 requirement and include path.

Or install a package:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DHTMLPARSER_BUILD_TESTS=OFF -DHTMLPARSER_BUILD_EXAMPLES=OFF
cmake --build build --parallel
cmake --install build --prefix /path/to/install
```

```cmake
find_package(HtmlParser 0.2 CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE HtmlParser::HtmlParser)
```

Set `CMAKE_PREFIX_PATH` to the installation prefix. `BUILD_SHARED_LIBS=ON` builds a shared library.

## Documents and fragments

`Parse` creates a document containing `html`, `head`, and `body`, including for empty input. `ParseFragment` returns the supplied nodes directly beneath a document root:

```cpp
auto Fragment = HtmlParser::ParseFragment("<p>One</p><p>Two</p>");
std::cout << Fragment.ToHtml(); // <p>One</p><p>Two</p>

auto Rows = HtmlParser::ParseFragment("<tr><td>Cell", "table");
std::cout << Rows.ToHtml(); // <tbody><tr><td>Cell</td></tr></tbody>
```

The optional context tag also controls raw text and RCDATA decoding, such as for `script` and `textarea`. Input is UTF-8; there is no encoding detection or network fetching.

## Querying and editing

Query directly on a `DOM` or `Node`. Results include the query root when it is an element and matches; all results remain within that subtree and appear once in document order. `SelectFirst` stops at the first match. `:scope` identifies an element query root.

```cpp
auto Document = HtmlParser::Parse("<main><p class=old>Before</p></main>");
if (auto Paragraph = Document.SelectFirst("main > p"))
{
    Paragraph->SetAttribute("class", "updated");
    Paragraph->SetTextContent("After & more");
    auto Container = Paragraph->Closest("main");
    std::cout << Paragraph->ToHtml(); // <p class="updated">After &amp; more</p>
}
```

`GetElementById`, `GetElementsByTagName`, and `GetElementsByClassName` remain available. Class lookup accepts multiple whitespace-separated classes and requires all of them.

For repeated queries, compile the selector once:

```cpp
HtmlParser::Selector Links("article a[href]:not(.disabled)");
if (!Links.IsValid())
    std::cerr << Links.Error() << '\n';

HtmlParser::Query Query(Document.Root());
auto Matches = Query.Select(Links);
auto First = Query.SelectFirst(Links);
```

Supported selectors:

| Category | Syntax |
| --- | --- |
| Basic | `p`, `*`, `#id`, `.class`, `p.one.two`, escaped identifiers |
| Relationships | descendant, `>`, `+`, `~`, comma-separated lists |
| Attributes | `[name]`, `=`, `~=`, `\|=`, `^=`, `$=`, `*=`, quoted values, `i`/`s` flags |
| Logical | `:not(...)`, `:is(...)`, `:where(...)` |
| Structural | `:scope`, `:root`, `:empty`, `:first-child`, `:last-child`, `:only-child`, corresponding `-of-type` forms |
| Positions | `:nth-child(...)`, `:nth-last-child(...)`, `:nth-of-type(...)`, `:nth-last-of-type(...)`; integers, `odd`, `even`, `An+B` |

Invalid or unsupported selectors produce an empty result. Use `Selector::Error()` to inspect the reason. Nested functional selectors are limited to 32 levels.

## Errors and limits

A parser can be reused without retaining the previous document. Its parse methods are `const` and support concurrent calls with fixed options:

```cpp
HtmlParser::Parser Parser({.MaxDepth = 1024, .MaxNodes = 100000});
auto Result = Parser.ParseWithDiagnostics("<div>\n</unexpected></div>");
for (const auto& Error : Result.Errors)
    std::cerr << Error.Line << ':' << Error.Column << ' ' << Error.Message << '\n';
```

`ParseFragmentWithDiagnostics` provides the same diagnostics for fragments. `SetStrict(true)` or `ParseOptions{.Strict = true}` makes recognized errors throw `ParseException`, a `std::runtime_error` subclass. Strict mode checks explicit nesting and recognized tokenizer errors; it is not an HTML conformance validator.

| Option | Default | Meaning |
| --- | --- | --- |
| `Strict` | `false` | Throw on recognized errors instead of recovering |
| `MaxDepth` | `4096` | Maximum element nesting, including implicit document containers; `0` disables the limit |
| `MaxNodes` | `1000000` | Maximum nodes including the document, comments, and text; `0` disables the limit |
| `MaxErrors` | `100` | Maximum diagnostics returned by a diagnostic parse |

Resource limit violations always throw. Error offsets are zero-based byte offsets into the original input; lines and byte columns are one-based. CRLF counts as one newline. The standalone tokenizer retains at most 100 diagnostics.

## Ownership

The public DOM retains shared node handles for compatibility. Holding a node keeps its subtree alive after the `DOM` is released; `Parent` is weak. Internal parsing and read-only traversal use borrowed pointers instead of copying ownership at every step.

Use `AppendChild`, `InsertBefore`, `RemoveChild`, and `Remove` to edit tree structure. Insertion detaches an existing parent, supports reordering, and rejects null children, document children, and cycles. `SetTextContent` detaches replaced children. Direct writes to public `Children`, `Parent`, or `Attributes` bypass these guarantees; attribute methods normalize HTML attribute names to ASCII lowercase.

A `const DOM` still refers to mutable nodes. Concurrent reads are supported while the tree is unchanged; mutation requires external synchronization. Traversal, text extraction, serialization, and destruction avoid recursion through the DOM.

## Supported HTML behavior

The parser handles document containers and head content, comments and doctypes, the complete named HTML character-reference table, numeric references and replacement rules, duplicate attributes with first-value precedence, raw-text elements, RCDATA, void elements, common omitted end tags, and implicit table row containers. Serialization escapes normal text and attributes, preserves raw text, sorts attributes for deterministic output, and preserves preformatted leading newlines across round trips.

Tree construction is a practical subset of the [HTML parsing standard](https://html.spec.whatwg.org/multipage/parsing.html). Full browser conformance remains unfinished: the adoption-agency algorithm, table foster parenting, foreign-content namespaces, template insertion modes, and escaped/double-escaped script states are not implemented. CSS namespaces, pseudo-elements, `:has()`, `nth-child(... of ...)`, and forgiving selector-list recovery are unsupported. HTML parsing and serialization do not sanitize content.

## Development

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Standalone builds enable tests and examples. CMake uses an installed GoogleTest 1.12+ when available, otherwise downloads the pinned test dependency. Embedded and installed consumers do not need GoogleTest.

For sanitizers with GCC or Clang, set `HTMLPARSER_ENABLE_SANITIZERS=ON`.

[Migration notes](docs/Migration.md) cover behavior changes. The [examples](examples) include fragments, diagnostics, querying, traversal, and mutation. The checked-in entity table is derived from Python's WHATWG-based `html.entities.html5` data; Python is not required to build or use the library.
