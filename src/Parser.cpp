#include <HtmlParser/Parser.hpp>

#include <algorithm>
#include <initializer_list>
#include <utility>

#include "Utilities.hpp"

namespace HtmlParser
{
    namespace
    {
        bool IsOneOf(std::string_view Tag, std::initializer_list<std::string_view> Tags)
        {
            return std::find(Tags.begin(), Tags.end(), Tag) != Tags.end();
        }

        bool IsWhitespace(std::string_view Text)
        {
            return std::all_of(Text.begin(), Text.end(), Utils::IsWhitespace);
        }

        class TreeBuilder
        {
        public:
            TreeBuilder(std::string_view Source, Tokenizer& Tokens, const ParseOptions& Options, bool Fragment,
                std::string_view ContextTag, bool CollectErrors)
                : m_Source(Source), m_Tokens(Tokens), m_Options(Options), m_Fragment(Fragment), m_CollectErrors(CollectErrors)
            {
                m_Open.push_back(m_Document.Root().get());
                if (Fragment)
                {
                    m_Context = std::make_shared<Node>(NodeType::Element);
                    m_Context->Tag = Utils::ToLower(std::string(ContextTag));
                    m_Open.push_back(m_Context.get());
                    m_Mode = Mode::InBody;
                    m_Tokens.SetTextMode(m_Context->Tag);
                }
            }

            ParseResult Build()
            {
                std::size_t LexicalErrors = 0;
                for (;;)
                {
                    auto Current = m_Tokens.NextToken();
                    const auto& Errors = m_Tokens.GetErrors();
                    while (LexicalErrors < Errors.size())
                        Report(Errors[LexicalErrors++]);
                    m_Offset = Current.Offset;
                    if (Current.Type == TokenType::EOFToken)
                        break;
                    Process(Current);
                }
                if (!m_Fragment)
                {
                    Token End;
                    // Reprocessing EOF creates all missing document containers.
                    Process(End);
                }
                for (std::size_t Index = m_Fragment ? 2 : 1; Index < m_Open.size(); ++Index)
                    if (!IsOneOf(m_Open[Index]->Tag, {"html", "head", "body"}))
                        Error("Unclosed element: " + m_Open[Index]->Tag);
                if (m_Fragment)
                {
                    auto Root = m_Document.Root();
                    Root->Children = std::move(m_Context->Children);
                    for (const auto& Child : Root->Children)
                        Child->Parent = Root;
                }
                return {std::move(m_Document), std::move(m_Errors)};
            }

        private:
            enum class Mode { Initial, BeforeHtml, BeforeHead, InHead, AfterHead, InBody, AfterBody, AfterHtml };

            Node* Current() const { return m_Open.back(); }
            std::size_t Floor() const { return m_Fragment ? 2 : 1; }

            void Report(const ParseError& Error)
            {
                if (m_Options.Strict)
                    throw ParseException(Error);
                if (m_CollectErrors && m_Errors.size() < m_Options.MaxErrors)
                    m_Errors.push_back(Error);
            }

            void Error(std::string Message)
            {
                if (m_Options.Strict || (m_CollectErrors && m_Errors.size() < m_Options.MaxErrors))
                    Report(Utils::MakeError(m_Source, m_Offset, std::move(Message)));
            }

            std::shared_ptr<Node> Create(NodeType Type)
            {
                if (m_Options.MaxNodes && m_NodeCount >= m_Options.MaxNodes)
                    throw ParseException(Utils::MakeError(m_Source, m_Offset, "Maximum node count exceeded"));
                ++m_NodeCount;
                return std::make_shared<Node>(Type);
            }

            void Attach(Node* Parent, const std::shared_ptr<Node>& Child)
            {
                Child->Parent = Parent->weak_from_this();
                Parent->Children.push_back(Child);
            }

            Node* InsertElement(Token& CurrentToken)
            {
                const bool Push = !Utils::IsVoidElement(CurrentToken.Data);
                const auto Depth = m_Open.size() - (m_Fragment ? 2 : 1);
                if (m_Options.MaxDepth && Depth >= m_Options.MaxDepth)
                    throw ParseException(Utils::MakeError(m_Source, m_Offset, "Maximum nesting depth exceeded"));
                auto Element = Create(NodeType::Element);
                Element->Tag = std::move(CurrentToken.Data);
                Element->Attributes = std::move(CurrentToken.Attributes);
                Attach(Current(), Element);
                if (Push)
                {
                    // HTML ignores the self-closing flag on non-void elements.
                    if (CurrentToken.SelfClosing)
                        Error("Self-closing syntax on non-void element: " + Element->Tag);
                    m_Open.push_back(Element.get());
                    if (Element->Tag == "p")
                        ++m_ParagraphCount;
                }
                return Element.get();
            }

            Node* InsertImplicit(std::string_view Tag)
            {
                Token Element;
                Element.Type = TokenType::StartTag;
                Element.Data = Tag;
                return InsertElement(Element);
            }

            void InsertText(Token& CurrentToken)
            {
                if (CurrentToken.Data.empty())
                    return;
                if (!Current()->Children.empty() && Current()->Children.back()->Type == NodeType::Text)
                    Current()->Children.back()->Text += CurrentToken.Data;
                else
                {
                    auto Text = Create(NodeType::Text);
                    Text->Text = std::move(CurrentToken.Data);
                    Attach(Current(), Text);
                }
            }

            void InsertComment(Token& CurrentToken, Node* Parent)
            {
                auto Comment = Create(NodeType::Comment);
                Comment->Text = std::move(CurrentToken.Data);
                Attach(Parent, Comment);
            }

            std::size_t FindOpen(std::string_view Tag) const
            {
                for (auto Index = m_Open.size(); Index > Floor(); --Index)
                    if (m_Open[Index - 1]->Tag == Tag)
                        return Index - 1;
                return 0;
            }

            void PopTo(std::size_t Size)
            {
                while (m_Open.size() > Size)
                {
                    if (m_Open.back()->Tag == "p")
                        --m_ParagraphCount;
                    m_Open.pop_back();
                }
            }

            void Close(std::string_view Tag)
            {
                const auto Index = FindOpen(Tag);
                if (!Index)
                {
                    Error("No matching start tag for end tag: " + std::string(Tag));
                    return;
                }
                if (Index + 1 != m_Open.size())
                    Error("Unexpected end tag: " + std::string(Tag));
                PopTo(Index);
            }

            void CloseOptional(std::initializer_list<std::string_view> Tags, std::initializer_list<std::string_view> Boundaries)
            {
                if (Tags.size() == 1 && *Tags.begin() == "p" && m_ParagraphCount == 0)
                    return;
                for (auto Index = m_Open.size(); Index > Floor(); --Index)
                {
                    const auto& Tag = m_Open[Index - 1]->Tag;
                    if (IsOneOf(Tag, Tags))
                    {
                        PopTo(Index - 1);
                        return;
                    }
                    if (IsOneOf(Tag, Boundaries))
                        return;
                }
            }

            void MergeAttributes(Node* Destination, Token& CurrentToken)
            {
                if (Destination)
                    for (auto& Attribute : CurrentToken.Attributes)
                        Destination->Attributes.emplace(Attribute.first, std::move(Attribute.second));
            }

            void StartBody(Token& CurrentToken)
            {
                const auto& Tag = CurrentToken.Data;
                if (!m_Fragment && Tag == "html")
                {
                    MergeAttributes(m_Html, CurrentToken);
                    return;
                }
                if (!m_Fragment && Tag == "body")
                {
                    MergeAttributes(m_Body, CurrentToken);
                    return;
                }
                if (!m_Fragment && Tag == "head")
                {
                    Error("Unexpected head element in body");
                    return;
                }
                if (Tag == "li")
                    CloseOptional({"li"}, {"ul", "ol"});
                if (Tag == "dt" || Tag == "dd")
                    CloseOptional({"dt", "dd"}, {"dl"});
                if (Tag == "option")
                    CloseOptional({"option"}, {"select", "datalist"});
                if (Tag == "optgroup")
                {
                    CloseOptional({"option"}, {"select"});
                    CloseOptional({"optgroup"}, {"select"});
                }
                if (IsOneOf(Tag, {"address", "article", "aside", "blockquote", "div", "dl", "fieldset", "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6", "header", "hr", "main", "menu", "nav", "ol", "p", "pre", "section", "table", "ul"}))
                    CloseOptional({"p"}, {"button", "table", "td", "th"});
                if (IsOneOf(Tag, {"h1", "h2", "h3", "h4", "h5", "h6"}) && IsOneOf(Current()->Tag, {"h1", "h2", "h3", "h4", "h5", "h6"}))
                    m_Open.pop_back();
                if (Tag == "a" && FindOpen("a"))
                    Close("a");
                if (Tag == "tr")
                {
                    CloseOptional({"td", "th"}, {"table"});
                    CloseOptional({"tr"}, {"table"});
                    if (Current()->Tag == "table")
                        InsertImplicit("tbody");
                }
                if (Tag == "td" || Tag == "th")
                {
                    CloseOptional({"td", "th"}, {"tr", "table"});
                    if (Current()->Tag == "table")
                        InsertImplicit("tbody");
                    if (IsOneOf(Current()->Tag, {"tbody", "thead", "tfoot"}))
                        InsertImplicit("tr");
                }
                if (IsOneOf(Tag, {"tbody", "thead", "tfoot"}))
                {
                    CloseOptional({"td", "th"}, {"table"});
                    CloseOptional({"tr"}, {"table"});
                    CloseOptional({"tbody", "thead", "tfoot"}, {"table"});
                }
                if (Tag == "col" && Current()->Tag == "table")
                    InsertImplicit("colgroup");
                InsertElement(CurrentToken);
            }

            void ProcessBody(Token& CurrentToken)
            {
                if (CurrentToken.Type == TokenType::Character)
                {
                    if (m_SkipLeadingNewline)
                    {
                        if (!CurrentToken.Data.empty() && CurrentToken.Data.front() == '\n')
                            CurrentToken.Data.erase(0, 1);
                        m_SkipLeadingNewline = false;
                    }
                    InsertText(CurrentToken);
                }
                else if (CurrentToken.Type == TokenType::StartTag)
                {
                    m_SkipLeadingNewline = IsOneOf(CurrentToken.Data, {"pre", "textarea", "listing"});
                    StartBody(CurrentToken);
                }
                else if (CurrentToken.Type == TokenType::EndTag)
                {
                    m_SkipLeadingNewline = false;
                    const auto& Tag = CurrentToken.Data;
                    if (!m_Fragment && (Tag == "body" || Tag == "html"))
                    {
                        if (!m_Body)
                        {
                            Error("Unexpected end tag in head: " + Tag);
                            return;
                        }
                        if (m_Open.size() > 3)
                            Error("Unclosed element before end of body");
                        PopTo(2);
                        m_Mode = Tag == "body" ? Mode::AfterBody : Mode::AfterHtml;
                    }
                    else if (m_Mode == Mode::InHead && Tag == "head")
                        Error("Unexpected head end tag inside head content");
                    else
                        Close(Tag);
                }
                else if (CurrentToken.Type == TokenType::DOCTYPE)
                    Error("Unexpected doctype");
            }

            void Process(Token& CurrentToken)
            {
                const auto Type = CurrentToken.Type;
                if (Type == TokenType::Comment)
                {
                    const bool Outside = m_Mode == Mode::Initial || m_Mode == Mode::BeforeHtml || m_Mode == Mode::AfterHtml;
                    m_SkipLeadingNewline = false;
                    InsertComment(CurrentToken, Outside ? m_Document.Root().get() : Current());
                    return;
                }
                for (;;)
                {
                    const auto& Tag = CurrentToken.Data;
                    const bool Whitespace = Type == TokenType::Character && IsWhitespace(Tag);
                    switch (m_Mode)
                    {
                    case Mode::Initial:
                        if (Whitespace)
                            return;
                        m_Mode = Mode::BeforeHtml;
                        if (Type == TokenType::DOCTYPE)
                        {
                            auto Doctype = Create(NodeType::Doctype);
                            Doctype->Text = Tag.empty() ? "html" : std::move(CurrentToken.Data);
                            Attach(m_Document.Root().get(), Doctype);
                            return;
                        }
                        continue;
                    case Mode::BeforeHtml:
                        if (Whitespace)
                            return;
                        if (Type == TokenType::StartTag && Tag == "html")
                        {
                            m_Html = InsertElement(CurrentToken);
                            m_Mode = Mode::BeforeHead;
                            return;
                        }
                        m_Html = InsertImplicit("html");
                        m_Mode = Mode::BeforeHead;
                        continue;
                    case Mode::BeforeHead:
                        if (Whitespace)
                            return;
                        if (Type == TokenType::StartTag && Tag == "html")
                        {
                            MergeAttributes(m_Html, CurrentToken);
                            return;
                        }
                        if (Type == TokenType::StartTag && Tag == "head")
                        {
                            m_Head = InsertElement(CurrentToken);
                            m_Mode = Mode::InHead;
                            return;
                        }
                        m_Head = InsertImplicit("head");
                        m_Mode = Mode::InHead;
                        continue;
                    case Mode::InHead:
                        if (Current() != m_Head)
                        {
                            if (Type != TokenType::EOFToken && !(Type == TokenType::EndTag && Tag == "head"))
                            {
                                ProcessBody(CurrentToken);
                                return;
                            }
                            Error("Unclosed element: " + Current()->Tag);
                            PopTo(3);
                        }
                        if (Whitespace)
                            return;
                        if (Type == TokenType::StartTag && IsOneOf(Tag, {"base", "basefont", "bgsound", "link", "meta", "title", "style", "script", "noframes", "template"}))
                        {
                            InsertElement(CurrentToken);
                            return;
                        }
                        if (Type == TokenType::EndTag && Tag == "head")
                        {
                            m_Open.pop_back();
                            m_Mode = Mode::AfterHead;
                            return;
                        }
                        m_Open.pop_back();
                        m_Mode = Mode::AfterHead;
                        continue;
                    case Mode::AfterHead:
                        if (Whitespace)
                            return;
                        if (Type == TokenType::StartTag && Tag == "body")
                        {
                            m_Body = InsertElement(CurrentToken);
                            m_Mode = Mode::InBody;
                            return;
                        }
                        m_Body = InsertImplicit("body");
                        m_Mode = Mode::InBody;
                        continue;
                    case Mode::AfterBody:
                    case Mode::AfterHtml:
                        if (Type == TokenType::EOFToken)
                            return;
                        if (Type == TokenType::EndTag && Tag == "html")
                        {
                            m_Mode = Mode::AfterHtml;
                            return;
                        }
                        PopTo(2);
                        m_Open.push_back(m_Body);
                        m_Mode = Mode::InBody;
                        continue;
                    case Mode::InBody:
                        ProcessBody(CurrentToken);
                        return;
                    }
                }
            }

            std::string_view m_Source;
            Tokenizer& m_Tokens;
            const ParseOptions& m_Options;
            bool m_Fragment;
            bool m_CollectErrors;
            DOM m_Document;
            std::shared_ptr<Node> m_Context;
            std::vector<Node*> m_Open;
            Node* m_Html = nullptr;
            Node* m_Head = nullptr;
            Node* m_Body = nullptr;
            Mode m_Mode = Mode::Initial;
            bool m_SkipLeadingNewline = false;
            std::size_t m_NodeCount = 1;
            std::size_t m_ParagraphCount = 0;
            std::size_t m_Offset = 0;
            std::vector<ParseError> m_Errors;
        };
    }

    DOM Parser::Parse(std::string_view Input) const
    {
        Tokenizer Tokens(Input, true);
        return TreeBuilder(Input, Tokens, m_Options, false, {}, false).Build().Document;
    }

    DOM Parser::ParseFragment(std::string_view Input, std::string_view ContextTag) const
    {
        Tokenizer Tokens(Input, true);
        return TreeBuilder(Input, Tokens, m_Options, true, ContextTag, false).Build().Document;
    }

    ParseResult Parser::ParseWithDiagnostics(std::string_view Input) const
    {
        Tokenizer Tokens(Input, true);
        return TreeBuilder(Input, Tokens, m_Options, false, {}, true).Build();
    }

    ParseResult Parser::ParseFragmentWithDiagnostics(std::string_view Input, std::string_view ContextTag) const
    {
        Tokenizer Tokens(Input, true);
        return TreeBuilder(Input, Tokens, m_Options, true, ContextTag, true).Build();
    }
} // namespace HtmlParser
