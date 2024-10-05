#include <gtest/gtest.h>

#include <HtmlParser/Parser.hpp>
#include <HtmlParser/Query.hpp>

TEST(QueryTest, FindBySelector)
{
    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse("<html><body><p>Hello World</p></body></html>");
    HtmlParser::Query Query;
    auto Result = Query.FindAll(DOM, "p");
    ASSERT_EQ(Result.size(), 1);
    ASSERT_EQ(Result[0]->Text, "Hello World");
}
