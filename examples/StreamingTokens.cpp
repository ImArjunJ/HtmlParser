#include <HtmlParser/Tokenizer.hpp>
#include <iostream>

int main()
{
    HtmlParser::Tokenizer Tokens("<p>A &amp; B</p><script>x < y</script>");
    for (auto Token = Tokens.NextToken(); Token.Type != HtmlParser::TokenType::EOFToken; Token = Tokens.NextToken())
        std::cout << Token.Offset << ':' << Token.EndOffset << ' ' << Token.Data << '\n';
}
