# Migration notes

Normal calls to `Parser::Parse`, DOM lookups, `Query::Select`, and existing node operations remain source compatible. The public `shared_ptr<Node>` handles and fields remain available. Rebuild dependents: this release changes class layouts and method signatures, so it does not preserve binary compatibility or exact pointer-to-member function types.

- Parsing accepts `std::string_view`, string literals, and strings. Input is borrowed only during a synchronous parser call. The standalone `Tokenizer` owns its input, including temporary strings.
- Parser state belongs to each call. A parser no longer holds the previously parsed document alive. Keep a `DOM` or node handle for content you need later.
- Empty input now creates a complete `html`/`head`/`body` document. Use `ParseFragment` for output without document containers.
- Text beginning with whitespace is preserved instead of being discarded with the whole text run. Newlines normalize CRLF and CR to LF, and input nulls become U+FFFD.
- `script`, `style`, and other raw-text elements preserve literal markup and ampersands. `title` and `textarea` preserve literal markup while decoding references.
- Duplicate attributes keep the first value. Tag and attribute names use ASCII case folding, independent of the process locale. Attribute method calls also normalize names.
- Numeric references accept omitted semicolons and apply HTML replacement rules. The complete named-reference table replaces the earlier small set. Ambiguous semicolon-free references in attributes remain literal where required.
- Common omitted end tags now close before subsequent list items, paragraphs, options, and table cells. Direct `tr` children of a `table` receive an implicit `tbody`.
- Self-closing syntax on HTML non-void elements is ignored in recovery mode and reported as an error. Use a closing tag for `<div>`, `<script>`, and other non-void HTML elements.
- Query results are unique and follow document order, including selector lists. Invalid selectors return no matches; a compiled `Selector` exposes the error. Attribute equality requires the attribute to exist, even for an empty value.
- Query roots are inclusive. Ancestor and sibling conditions remain bounded by a query's subtree; `Node::Matches` and `Closest` can inspect ancestors outside it.
- `ToHtml` sorts attributes and serializes an element root itself. Output strings may change even when the document is equivalent.
- Tree mutation detaches prior parents and rejects invalid structures. `GetTextContent` on a comment returns its own text; container text still excludes comments and doctypes.
- Resource limits default to 4,096 element levels and 1,000,000 nodes. Configure `ParseOptions` for larger documents. `0` disables a limit.
- Strict mode reports recognized lexical errors as well as nesting errors. Catch `ParseException` for source position details; existing `std::runtime_error` catches still work. Strict mode requires explicit closure of non-container elements, including elements whose end tags HTML may allow you to omit.
- `BUILD_SHARED_LIBS` selects shared or static linkage. Installation provides the `HtmlParser::HtmlParser` imported target.

`Tokenizer::NextToken` provides incremental consumption without retaining a token vector. `Tokenize` resets the tokenizer and populates `GetTokens` for existing callers; that vector excludes EOF. `Reset` clears tokens, diagnostics, and text mode. Token `Offset` and `EndOffset` describe the original input's half-open byte range.
