#pragma once
#include <string>

#include "DOM.hpp"
#include "Tokenizer.hpp"

namespace HtmlParser
{
    class Parser
    {
    public:
        Parser();

        DOM Parse(const std::string& Input);

        void SetStrict(bool Strict)
        {
            IsStrict = Strict;
        }

    private:
        void InsertionModeInitial(const Token& Token);
        void InsertionModeBeforeHtml(const Token& Token);
        void InsertionModeBeforeHead(const Token& Token);
        void InsertionModeInHead(const Token& Token);
        void InsertionModeAfterHead(const Token& Token);
        void InsertionModeInBody(const Token& Token);

        void HandleError(const std::string& Message);

        std::shared_ptr<Node> CurrentNode();

        void InsertElement(const Token& Token);
        void InsertCharacter(const Token& Token);
        void CloseElement(const Token& Token);

        std::shared_ptr<Node> Document;
        std::vector<std::shared_ptr<Node>> OpenElements;

        enum class InsertionMode
        {
            Initial,
            BeforeHtml,
            BeforeHead,
            InHead,
            AfterHead,
            InBody,
            Text,
        };

        InsertionMode InsertionMode;

        bool IsStrict = false;
    };
} // namespace HtmlParser
