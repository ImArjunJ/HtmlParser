#include <HtmlParser/Query.hpp>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

#include "Tree.hpp"
#include "Utilities.hpp"

namespace HtmlParser
{
    namespace
    {
        struct Group;
        enum class Kind { Id, Class, Attribute, Scope, Root, Empty, First, Last, Only, FirstType, LastType, OnlyType,
            Nth, NthLast, NthType, NthLastType, Not, Is };
        enum class Operator { Exists, Equal, Word, Dash, Prefix, Suffix, Substring };
        struct Test
        {
            Kind Type = Kind::Attribute;
            std::string Name;
            std::string Value;
            Operator Operation = Operator::Exists;
            bool Insensitive = false;
            int A = 0;
            int B = 0;
            std::vector<Group> Nested;
        };
        struct Compound { std::string Tag; std::vector<Test> Tests; };
        struct Step { Compound Filter; char Relation = ' '; };
        struct Group { std::vector<Step> Steps; };

        class SelectorParser
        {
        public:
            explicit SelectorParser(std::string_view Source) : m_Source(Source) {}
            std::vector<Group> Parse()
            {
                auto Result = List(0);
                SkipWhitespace();
                if (m_Position != m_Source.size())
                    Fail("Unexpected selector input");
                return m_Error.empty() ? Result : std::vector<Group>{};
            }
            std::string Error() const { return m_Error; }

        private:
            char Peek() const { return m_Position < m_Source.size() ? m_Source[m_Position] : '\0'; }
            void Fail(std::string Message)
            {
                if (m_Error.empty())
                    m_Error = std::move(Message) + " at byte " + std::to_string(m_Position);
            }
            bool SkipWhitespace()
            {
                const auto Start = m_Position;
                while (Utils::IsWhitespace(Peek()))
                    ++m_Position;
                return Start != m_Position;
            }
            static bool NameCharacter(char Value)
            {
                return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z') ||
                    (Value >= '0' && Value <= '9') || Value == '-' || Value == '_' || static_cast<unsigned char>(Value) >= 0x80;
            }
            void Escape(std::string& Result)
            {
                ++m_Position;
                if (!Peek() || Peek() == '\n' || Peek() == '\r' || Peek() == '\f')
                {
                    Fail("Incomplete escape");
                    return;
                }
                unsigned int Codepoint = 0;
                int Count = 0;
                for (; Count < 6; ++Count)
                {
                    const auto Value = Utils::Lower(Peek());
                    const int Digit = Value >= '0' && Value <= '9' ? Value - '0' : Value >= 'a' && Value <= 'f' ? Value - 'a' + 10 : -1;
                    if (Digit < 0)
                        break;
                    Codepoint = Codepoint * 16 + static_cast<unsigned int>(Digit);
                    ++m_Position;
                }
                if (!Count)
                    Result += m_Source[m_Position++];
                else
                {
                    if (Utils::IsWhitespace(Peek()))
                    {
                        const char Previous = m_Source[m_Position++];
                        if (Previous == '\r' && Peek() == '\n')
                            ++m_Position;
                    }
                    if (!Codepoint || Codepoint > 0x10FFFF || (Codepoint >= 0xD800 && Codepoint <= 0xDFFF))
                        Codepoint = 0xFFFD;
                    Utils::AppendCodepointUtf8(Result, Codepoint);
                }
            }
            std::string Identifier()
            {
                std::string Result;
                while (NameCharacter(Peek()) || Peek() == '\\')
                {
                    if (Peek() == '\\')
                        Escape(Result);
                    else
                        Result += m_Source[m_Position++];
                    if (!m_Error.empty())
                        break;
                }
                if (Result.empty())
                    Fail("Expected identifier");
                return Result;
            }
            std::string Value()
            {
                if (Peek() != '\'' && Peek() != '"')
                    return Identifier();
                const auto Quote = m_Source[m_Position++];
                std::string Result;
                while (Peek() && Peek() != Quote)
                {
                    if (Peek() == '\\')
                        Escape(Result);
                    else if (Peek() == '\n' || Peek() == '\r' || Peek() == '\f')
                    {
                        Fail("Newline in string");
                        break;
                    }
                    else
                        Result += m_Source[m_Position++];
                    if (!m_Error.empty())
                        return Result;
                }
                if (Peek() != Quote)
                    Fail("Unterminated string");
                else
                    ++m_Position;
                return Result;
            }
            Test Attribute()
            {
                Test Result;
                ++m_Position;
                SkipWhitespace();
                Result.Name = Utils::ToLower(Identifier());
                SkipWhitespace();
                if (Peek() != ']')
                {
                    switch (Peek())
                    {
                    case '=': Result.Operation = Operator::Equal; break;
                    case '~': Result.Operation = Operator::Word; break;
                    case '|': Result.Operation = Operator::Dash; break;
                    case '^': Result.Operation = Operator::Prefix; break;
                    case '$': Result.Operation = Operator::Suffix; break;
                    case '*': Result.Operation = Operator::Substring; break;
                    default: Fail("Invalid attribute operator"); return Result;
                    }
                    if (Peek() != '=')
                        ++m_Position;
                    if (Peek() != '=')
                    {
                        Fail("Expected equals sign");
                        return Result;
                    }
                    ++m_Position;
                    SkipWhitespace();
                    Result.Value = Value();
                    SkipWhitespace();
                    if (Peek() == 'i' || Peek() == 'I' || Peek() == 's' || Peek() == 'S')
                    {
                        Result.Insensitive = Utils::Lower(m_Source[m_Position++]) == 'i';
                        SkipWhitespace();
                    }
                }
                if (Peek() != ']')
                    Fail("Unterminated attribute selector");
                else
                    ++m_Position;
                return Result;
            }
            bool Integer(std::string_view Input, int& Output)
            {
                if (!Input.empty() && Input.front() == '+')
                    Input.remove_prefix(1);
                const auto Result = std::from_chars(Input.data(), Input.data() + Input.size(), Output);
                return Result.ec == std::errc{} && Result.ptr == Input.data() + Input.size();
            }
            void Nth(Test& Result)
            {
                const auto Start = m_Position;
                while (Peek() && Peek() != ')')
                    ++m_Position;
                std::string Expression;
                for (auto Value : m_Source.substr(Start, m_Position - Start))
                    if (!Utils::IsWhitespace(Value))
                        Expression += Utils::Lower(Value);
                if (Expression == "odd" || Expression == "even")
                {
                    Result.A = 2;
                    Result.B = Expression == "odd" ? 1 : 0;
                    return;
                }
                const auto N = Expression.find('n');
                if (N == std::string::npos)
                {
                    if (!Integer(Expression, Result.B))
                        Fail("Invalid nth expression");
                    return;
                }
                const auto Before = std::string_view(Expression).substr(0, N);
                const auto After = std::string_view(Expression).substr(N + 1);
                if (Before.empty() || Before == "+" || Before == "-")
                    Result.A = Before == "-" ? -1 : 1;
                else if (!Integer(Before, Result.A))
                    Fail("Invalid nth coefficient");
                if (!After.empty() && ((After.front() != '+' && After.front() != '-') || !Integer(After, Result.B)))
                    Fail("Invalid nth offset");
            }
            Test Pseudo(std::size_t Depth)
            {
                ++m_Position;
                Test Result;
                const auto Name = Utils::ToLower(Identifier());
                static const std::unordered_map<std::string, Kind> Names = {
                    {"scope", Kind::Scope}, {"root", Kind::Root}, {"empty", Kind::Empty},
                    {"first-child", Kind::First}, {"last-child", Kind::Last}, {"only-child", Kind::Only},
                    {"first-of-type", Kind::FirstType}, {"last-of-type", Kind::LastType}, {"only-of-type", Kind::OnlyType},
                    {"nth-child", Kind::Nth}, {"nth-last-child", Kind::NthLast}, {"nth-of-type", Kind::NthType},
                    {"nth-last-of-type", Kind::NthLastType}, {"not", Kind::Not}, {"is", Kind::Is}, {"where", Kind::Is}
                };
                const auto Found = Names.find(Name);
                if (Found == Names.end())
                {
                    Fail("Unsupported pseudo-class");
                    return Result;
                }
                Result.Type = Found->second;
                const bool Functional = Result.Type >= Kind::Nth;
                if (Functional)
                {
                    if (Peek() != '(')
                    {
                        Fail("Expected pseudo-class argument");
                        return Result;
                    }
                    ++m_Position;
                    if (Result.Type == Kind::Not || Result.Type == Kind::Is)
                        Result.Nested = List(Depth + 1);
                    else
                        Nth(Result);
                    if (Peek() != ')')
                        Fail("Unterminated pseudo-class");
                    else
                        ++m_Position;
                }
                return Result;
            }
            Compound Filter(std::size_t Depth)
            {
                Compound Result;
                if (Peek() == '*')
                    ++m_Position;
                else if (NameCharacter(Peek()) || Peek() == '\\')
                    Result.Tag = Utils::ToLower(Identifier());
                else if (Peek() != '.' && Peek() != '#' && Peek() != '[' && Peek() != ':')
                {
                    Fail("Expected selector");
                    return Result;
                }
                while (m_Error.empty())
                {
                    if (Peek() == '.' || Peek() == '#')
                    {
                        Test Item;
                        Item.Type = m_Source[m_Position++] == '.' ? Kind::Class : Kind::Id;
                        Item.Value = Identifier();
                        Result.Tests.push_back(std::move(Item));
                    }
                    else if (Peek() == '[')
                        Result.Tests.push_back(Attribute());
                    else if (Peek() == ':')
                        Result.Tests.push_back(Pseudo(Depth));
                    else
                        break;
                }
                return Result;
            }
            std::vector<Group> List(std::size_t Depth)
            {
                std::vector<Group> Result;
                if (Depth > 32)
                {
                    Fail("Selector nesting limit exceeded");
                    return Result;
                }
                do
                {
                    SkipWhitespace();
                    Group Current;
                    Current.Steps.push_back({Filter(Depth), ' '});
                    while (m_Error.empty())
                    {
                        const bool Space = SkipWhitespace();
                        if (!Peek() || Peek() == ')' || Peek() == ',')
                            break;
                        char Relation = ' ';
                        if (Peek() == '>' || Peek() == '+' || Peek() == '~')
                        {
                            Relation = m_Source[m_Position++];
                            SkipWhitespace();
                        }
                        else if (!Space)
                        {
                            Fail("Expected selector combinator");
                            break;
                        }
                        Current.Steps.push_back({Filter(Depth), Relation});
                    }
                    Result.push_back(std::move(Current));
                    if (Peek() != ',' || !m_Error.empty())
                        break;
                    ++m_Position;
                } while (m_Error.empty());
                return Result;
            }

            std::string_view m_Source;
            std::size_t m_Position = 0;
            std::string m_Error;
        };

        struct Siblings
        {
            std::vector<const Node*> Elements;
            std::unordered_map<const Node*, std::size_t> Positions;
            std::vector<std::size_t> TypePositions;
            std::unordered_map<std::string, std::size_t> TypeCounts;
        };
        struct MatchContext
        {
            const Node* Scope = nullptr;
            const Node* Boundary = nullptr;
            std::unordered_map<const Node*, Siblings> Cache;

            const Siblings& Family(const Node& Element)
            {
                auto Parent = Element.Parent.lock();
                const auto Key = Parent ? Parent.get() : &Element;
                auto [Entry, Inserted] = Cache.try_emplace(Key);
                if (Inserted)
                {
                    auto& Data = Entry->second;
                    auto Add = [&](const Node* Child) {
                        if (Child && Child->Type == NodeType::Element)
                        {
                            Data.Positions.emplace(Child, Data.Elements.size());
                            Data.Elements.push_back(Child);
                            Data.TypePositions.push_back(++Data.TypeCounts[Utils::ToLower(Child->Tag)]);
                        }
                    };
                    if (Parent)
                        for (const auto& Child : Parent->Children)
                            Add(Child.get());
                    else
                        Add(&Element);
                }
                return Entry->second;
            }
        };

        bool MatchList(const Node& Element, const std::vector<Group>& Groups, MatchContext& Context);

        bool MatchAttribute(const Node& Element, const Test& Rule)
        {
            const auto Found = Element.Attributes.find(Rule.Name);
            if (Found == Element.Attributes.end())
                return false;
            if (Rule.Operation == Operator::Exists)
                return true;
            const auto& Stored = Found->second;
            const auto LeftStorage = Rule.Insensitive ? Utils::ToLower(Stored) : std::string{};
            const auto RightStorage = Rule.Insensitive ? Utils::ToLower(Rule.Value) : std::string{};
            const std::string_view Left = Rule.Insensitive ? LeftStorage : Stored;
            const std::string_view Right = Rule.Insensitive ? RightStorage : Rule.Value;
            switch (Rule.Operation)
            {
            case Operator::Equal: return Left == Right;
            case Operator::Word: return Utils::ContainsWord(Left, Right);
            case Operator::Dash: return Left == Right || (Left.size() > Right.size() && Left.starts_with(Right) && Left[Right.size()] == '-');
            case Operator::Prefix: return !Right.empty() && Left.starts_with(Right);
            case Operator::Suffix: return !Right.empty() && Left.ends_with(Right);
            case Operator::Substring: return !Right.empty() && Left.find(Right) != std::string_view::npos;
            case Operator::Exists: return true;
            }
            return false;
        }

        bool MatchTest(const Node& Element, const Test& Rule, MatchContext& Context)
        {
            switch (Rule.Type)
            {
            case Kind::Id:
            {
                const auto Found = Element.Attributes.find("id");
                return Found != Element.Attributes.end() && Found->second == Rule.Value;
            }
            case Kind::Class:
            {
                const auto Found = Element.Attributes.find("class");
                return Found != Element.Attributes.end() && Utils::ContainsWord(Found->second, Rule.Value);
            }
            case Kind::Attribute: return MatchAttribute(Element, Rule);
            case Kind::Scope:
                if (Context.Scope && Context.Scope->Type == NodeType::Document)
                    return Element.Parent.lock().get() == Context.Scope;
                return &Element == Context.Scope;
            case Kind::Root:
            {
                auto Parent = Element.Parent.lock();
                return !Parent || Parent->Type == NodeType::Document;
            }
            case Kind::Empty:
                return std::none_of(Element.Children.begin(), Element.Children.end(), [](const auto& Child) {
                    return Child && (Child->Type == NodeType::Element || (Child->Type == NodeType::Text && !Child->Text.empty()));
                });
            case Kind::Not: return !MatchList(Element, Rule.Nested, Context);
            case Kind::Is: return MatchList(Element, Rule.Nested, Context);
            default: break;
            }
            const auto& Family = Context.Family(Element);
            const auto Found = Family.Positions.find(&Element);
            if (Found == Family.Positions.end())
                return false;
            const auto Position = Found->second;
            const bool OfType = Rule.Type == Kind::FirstType || Rule.Type == Kind::LastType || Rule.Type == Kind::OnlyType ||
                Rule.Type == Kind::NthType || Rule.Type == Kind::NthLastType;
            const auto Index = OfType ? Family.TypePositions[Position] : Position + 1;
            const auto Count = OfType ? Family.TypeCounts.at(Utils::ToLower(Element.Tag)) : Family.Elements.size();
            switch (Rule.Type)
            {
            case Kind::First: case Kind::FirstType: return Index == 1;
            case Kind::Last: case Kind::LastType: return Index == Count;
            case Kind::Only: case Kind::OnlyType: return Count == 1;
            default: break;
            }
            const bool Reverse = Rule.Type == Kind::NthLast || Rule.Type == Kind::NthLastType;
            const auto Value = static_cast<std::int64_t>(Reverse ? Count - Index + 1 : Index) - Rule.B;
            return Rule.A ? Value % Rule.A == 0 && Value / Rule.A >= 0 : Value == 0;
        }

        bool MatchFilter(const Node& Element, const Compound& Filter, MatchContext& Context)
        {
            if (Element.Type != NodeType::Element || (!Filter.Tag.empty() && !Utils::EqualsIgnoreCase(Element.Tag, Filter.Tag)))
                return false;
            return std::all_of(Filter.Tests.begin(), Filter.Tests.end(), [&](const auto& Rule) { return MatchTest(Element, Rule, Context); });
        }

        bool MatchGroup(const Node& Element, const Group& Pattern, MatchContext& Context)
        {
            if (!MatchFilter(Element, Pattern.Steps.back().Filter, Context))
                return false;
            if (Pattern.Steps.size() == 1)
                return true;
            struct State { const Node* Element; std::size_t Index; };
            std::vector<State> Pending{{&Element, Pattern.Steps.size() - 1}};
            while (!Pending.empty())
            {
                const auto [Current, Index] = Pending.back();
                Pending.pop_back();
                if (!MatchFilter(*Current, Pattern.Steps[Index].Filter, Context))
                    continue;
                if (!Index)
                    return true;
                if (Current == Context.Boundary)
                    continue;
                const auto Relation = Pattern.Steps[Index].Relation;
                if (Relation == '>' || Relation == ' ')
                {
                    auto Parent = Current->Parent.lock();
                    while (Parent)
                    {
                        Pending.push_back({Parent.get(), Index - 1});
                        if (Relation == '>' || Parent.get() == Context.Boundary)
                            break;
                        Parent = Parent->Parent.lock();
                    }
                }
                else
                {
                    const auto& Family = Context.Family(*Current);
                    auto Found = Family.Positions.find(Current);
                    if (Found != Family.Positions.end())
                        for (auto Position = Found->second; Position > 0; --Position)
                        {
                            Pending.push_back({Family.Elements[Position - 1], Index - 1});
                            if (Relation == '+')
                                break;
                        }
                }
            }
            return false;
        }

        bool MatchList(const Node& Element, const std::vector<Group>& Groups, MatchContext& Context)
        {
            return std::any_of(Groups.begin(), Groups.end(), [&](const auto& Pattern) { return MatchGroup(Element, Pattern, Context); });
        }
    }

    struct Selector::Impl { std::vector<Group> Groups; std::string Error; };
    Selector::Selector(std::string_view Pattern) : m_Impl(std::make_unique<Impl>())
    {
        SelectorParser Parser(Pattern);
        m_Impl->Groups = Parser.Parse();
        m_Impl->Error = Parser.Error();
    }
    Selector::~Selector() = default;
    Selector::Selector(const Selector& Other) : m_Impl(Other.m_Impl ? std::make_unique<Impl>(*Other.m_Impl) : nullptr) {}
    Selector& Selector::operator=(const Selector& Other)
    {
        if (this != &Other)
            m_Impl = Other.m_Impl ? std::make_unique<Impl>(*Other.m_Impl) : nullptr;
        return *this;
    }
    Selector::Selector(Selector&&) noexcept = default;
    Selector& Selector::operator=(Selector&&) noexcept = default;
    bool Selector::IsValid() const noexcept { return m_Impl && m_Impl->Error.empty() && !m_Impl->Groups.empty(); }
    std::string_view Selector::Error() const noexcept { return m_Impl ? std::string_view(m_Impl->Error) : std::string_view{}; }
    bool Selector::Matches(const Node& Element, const Node* Scope) const
    {
        MatchContext Context{Scope ? Scope : &Element, nullptr, {}};
        return IsValid() && MatchList(Element, m_Impl->Groups, Context);
    }

    Query::Query(const std::shared_ptr<Node>& QueryRoot) : m_Root(QueryRoot) {}
    std::vector<std::shared_ptr<Node>> Query::Select(std::string_view Pattern) const { return Select(Selector(Pattern)); }
    std::shared_ptr<Node> Query::SelectFirst(std::string_view Pattern) const { return SelectFirst(Selector(Pattern)); }
    std::vector<std::shared_ptr<Node>> Query::Select(const Selector& Pattern) const
    {
        std::vector<std::shared_ptr<Node>> Results;
        if (!Pattern.IsValid())
            return Results;
        MatchContext Context{m_Root.get(), m_Root.get(), {}};
        Detail::Walk(m_Root.get(), [&](auto* Current) {
            if (MatchList(*Current, Pattern.m_Impl->Groups, Context))
                Results.push_back(Current->shared_from_this());
            return true;
        });
        return Results;
    }
    std::shared_ptr<Node> Query::SelectFirst(const Selector& Pattern) const
    {
        std::shared_ptr<Node> Result;
        if (!Pattern.IsValid())
            return Result;
        MatchContext Context{m_Root.get(), m_Root.get(), {}};
        Detail::Walk(m_Root.get(), [&](auto* Current) {
            if (MatchList(*Current, Pattern.m_Impl->Groups, Context))
                Result = Current->shared_from_this();
            return !Result;
        });
        return Result;
    }
} // namespace HtmlParser
