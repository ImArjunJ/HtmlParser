#include <HtmlParser/Tokenizer.hpp>

#include "Utilities.hpp"

namespace HtmlParser
{
    namespace
    {
        bool IsAlpha(char Value)
        {
            return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z');
        }
    } // namespace

    Tokenizer::Tokenizer(std::string_view Input) : m_Storage(Input) {}
    Tokenizer::Tokenizer(std::string_view Input, bool Borrow) : m_View(Input), m_Borrowed(Borrow) {}
    std::string_view Tokenizer::Input() const noexcept
    {
        return m_Borrowed ? m_View : std::string_view(m_Storage);
    }
    const std::vector<Token>& Tokenizer::GetTokens() const noexcept
    {
        return m_Tokens;
    }
    const std::vector<ParseError>& Tokenizer::GetErrors() const noexcept
    {
        return m_Errors;
    }

    void Tokenizer::Reset()
    {
        m_Position = 0;
        m_TextTag.clear();
        m_DecodeText = true;
        m_Tokens.clear();
        m_Errors.clear();
    }

    void Tokenizer::Tokenize()
    {
        Reset();
        for (auto Current = NextToken(); Current.Type != TokenType::EOFToken; Current = NextToken())
            m_Tokens.push_back(std::move(Current));
    }

    void Tokenizer::SetTextMode(std::string_view Tag)
    {
        if (Utils::IsRawTextElement(Tag) || Utils::EqualsIgnoreCase(Tag, "title") ||
            Utils::EqualsIgnoreCase(Tag, "textarea"))
        {
            m_TextTag = Utils::ToLower(std::string(Tag));
            m_DecodeText = m_TextTag == "title" || m_TextTag == "textarea";
        }
    }

    bool Tokenizer::StartsWith(std::size_t Offset, std::string_view Value) const
    {
        return Offset <= Input().size() && Utils::EqualsIgnoreCase(Input().substr(Offset, Value.size()), Value);
    }

    void Tokenizer::Error(std::size_t Offset, std::string Message)
    {
        if (m_Errors.size() < 100)
            m_Errors.push_back(Utils::MakeError(Input(), Offset, std::move(Message)));
    }

    Token Tokenizer::NextToken()
    {
        const auto Source = Input();
        if (m_Position >= Source.size())
            return {TokenType::EOFToken, {}, {}, false, m_Position, m_Position};
        if (!m_TextTag.empty() || Source[m_Position] != '<')
            return ReadText();
        if (m_Position + 1 < Source.size())
        {
            if (Source[m_Position + 1] == '!' || Source[m_Position + 1] == '?')
                return ReadDeclaration();
            const auto Name = m_Position + (Source[m_Position + 1] == '/' ? 2 : 1);
            if (Name < Source.size() && IsAlpha(Source[Name]))
                return ReadTag();
        }
        return ReadText();
    }

    Token Tokenizer::ReadText()
    {
        const auto Source = Input();
        const auto Start = m_Position;
        const bool Decode = m_DecodeText;
        if (!m_TextTag.empty())
        {
            auto End = m_Position;
            if (m_TextTag != "plaintext")
            {
                while ((End = Source.find('<', End)) != std::string_view::npos)
                {
                    const auto AfterName = End + 2 + m_TextTag.size();
                    if (End + 1 < Source.size() && Source[End + 1] == '/' && StartsWith(End + 2, m_TextTag) &&
                        AfterName < Source.size() &&
                        (Utils::IsWhitespace(Source[AfterName]) || Source[AfterName] == '/' ||
                         Source[AfterName] == '>'))
                        break;
                    ++End;
                }
            }
            else
                End = std::string_view::npos;
            m_Position = End == std::string_view::npos ? Source.size() : End;
            if (End != std::string_view::npos)
            {
                m_TextTag.clear();
                m_DecodeText = true;
                if (m_Position == Start)
                    return ReadTag();
            }
        }
        else
        {
            auto Search = m_Position;
            for (;;)
            {
                const auto Next = Source.find('<', Search);
                if (Next == std::string_view::npos)
                {
                    m_Position = Source.size();
                    break;
                }
                auto Name = Next + 1;
                if (Name < Source.size() && (Source[Name] == '!' || Source[Name] == '?'))
                {
                    m_Position = Next;
                    break;
                }
                if (Name < Source.size() && Source[Name] == '/')
                    ++Name;
                if (Name < Source.size() && IsAlpha(Source[Name]))
                {
                    m_Position = Next;
                    break;
                }
                Search = Next + 1;
            }
        }
        auto Text = Utils::NormalizeText(Source.substr(Start, m_Position - Start));
        return {TokenType::Character,
                Decode && Text.find('&') != std::string::npos ? Utils::DecodeHtml(Text) : std::move(Text),
                {},
                false,
                Start,
                m_Position};
    }

    Token Tokenizer::ReadTag()
    {
        const auto Source = Input();
        Token Result;
        Result.Offset = m_Position++;
        Result.Type = TokenType::StartTag;
        if (Source[m_Position] == '/')
        {
            Result.Type = TokenType::EndTag;
            ++m_Position;
        }
        const auto NameStart = m_Position;
        while (m_Position < Source.size() && !Utils::IsWhitespace(Source[m_Position]) && Source[m_Position] != '/' &&
               Source[m_Position] != '>')
            ++m_Position;
        Result.Data = Utils::ToLower(Utils::NormalizeText(Source.substr(NameStart, m_Position - NameStart)));
        bool Closed = false;
        while (m_Position < Source.size())
        {
            while (m_Position < Source.size() && Utils::IsWhitespace(Source[m_Position]))
                ++m_Position;
            if (m_Position >= Source.size())
                break;
            if (Source[m_Position] == '>')
            {
                ++m_Position;
                Closed = true;
                break;
            }
            if (Source[m_Position] == '/')
            {
                ++m_Position;
                if (m_Position < Source.size() && Source[m_Position] == '>')
                {
                    ++m_Position;
                    Result.SelfClosing = true;
                    Closed = true;
                    break;
                }
                Error(m_Position - 1, "Unexpected slash in tag");
                continue;
            }
            const auto AttributeStart = m_Position;
            // An initial '=' is part of a malformed name, and must still be consumed.
            ++m_Position;
            while (m_Position < Source.size() && !Utils::IsWhitespace(Source[m_Position]) &&
                   Source[m_Position] != '=' && Source[m_Position] != '/' && Source[m_Position] != '>')
                ++m_Position;
            auto Name =
                Utils::ToLower(Utils::NormalizeText(Source.substr(AttributeStart, m_Position - AttributeStart)));
            while (m_Position < Source.size() && Utils::IsWhitespace(Source[m_Position]))
                ++m_Position;
            std::string Value;
            if (m_Position < Source.size() && Source[m_Position] == '=')
            {
                ++m_Position;
                while (m_Position < Source.size() && Utils::IsWhitespace(Source[m_Position]))
                    ++m_Position;
                const char Quote =
                    m_Position < Source.size() && (Source[m_Position] == '\'' || Source[m_Position] == '"')
                        ? Source[m_Position++]
                        : '\0';
                const auto ValueStart = m_Position;
                while (m_Position < Source.size() &&
                       (Quote ? Source[m_Position] != Quote
                              : !Utils::IsWhitespace(Source[m_Position]) && Source[m_Position] != '>'))
                    ++m_Position;
                Value =
                    Utils::DecodeHtml(Utils::NormalizeText(Source.substr(ValueStart, m_Position - ValueStart)), true);
                if (Quote && m_Position < Source.size())
                    ++m_Position;
            }
            if (Result.Type == TokenType::StartTag &&
                !Result.Attributes.emplace(std::move(Name), std::move(Value)).second)
                Error(AttributeStart, "Duplicate attribute");
        }
        Result.EndOffset = m_Position;
        if (!Closed)
        {
            Error(Result.Offset, "Unterminated tag");
            return {TokenType::EOFToken, {}, {}, false, m_Position, m_Position};
        }
        if (Result.Type == TokenType::StartTag)
            SetTextMode(Result.Data);
        return Result;
    }

    Token Tokenizer::ReadDeclaration()
    {
        const auto Source = Input();
        Token Result;
        Result.Offset = m_Position;
        Result.Type = TokenType::Comment;
        if (Source.substr(m_Position, 4) == "<!--")
        {
            const auto Start = m_Position + 4;
            auto End = Source.find("-->", Start);
            auto Alternate = Source.find("--!>", Start);
            if (Alternate != std::string_view::npos && (End == std::string_view::npos || Alternate < End))
            {
                End = Alternate;
                Error(End, "Incorrectly closed comment");
            }
            if (End == std::string_view::npos)
            {
                Error(Result.Offset, "Unterminated comment");
                Result.Data = Utils::NormalizeText(Source.substr(Start));
                m_Position = Source.size();
            }
            else
            {
                Result.Data = Utils::NormalizeText(Source.substr(Start, End - Start));
                m_Position = End + (End == Alternate ? 4 : 3);
            }
        }
        else
        {
            const bool Doctype = StartsWith(m_Position + 2, "doctype");
            const auto Start = m_Position + (Doctype ? 9 : 2);
            auto End = Start;
            char Quote = '\0';
            for (; End < Source.size(); ++End)
            {
                if (Doctype && (Source[End] == '\'' || Source[End] == '"'))
                {
                    if (!Quote)
                        Quote = Source[End];
                    else if (Source[End] == Quote)
                        Quote = '\0';
                }
                if (!Quote && Source[End] == '>')
                    break;
            }
            if (End == Source.size())
                Error(Result.Offset, "Unterminated declaration");
            Result.Type = Doctype ? TokenType::DOCTYPE : TokenType::Comment;
            Result.Data = Utils::NormalizeText(Source.substr(Start, End - Start));
            if (Doctype)
                Result.Data = Utils::Trim(Result.Data);
            else
                Error(Result.Offset, "Unknown declaration");
            m_Position = End < Source.size() ? End + 1 : End;
        }
        Result.EndOffset = m_Position;
        return Result;
    }
} // namespace HtmlParser
