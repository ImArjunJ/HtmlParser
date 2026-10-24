#include <HtmlParser/Node.hpp>
#include <HtmlParser/Query.hpp>

#include <algorithm>
#include <stdexcept>

#include "Tree.hpp"
#include "Utilities.hpp"

namespace HtmlParser
{
    Node::Node(NodeType _Type) : Type(_Type) {}

    Node::~Node()
    {
        // Release uniquely owned descendants without disturbing retained subtrees.
        auto Pending = std::move(Children);
        while (!Pending.empty())
        {
            auto Child = std::move(Pending.back());
            Pending.pop_back();
            if (Child && Child.use_count() == 1)
            {
                for (auto& Grandchild : Child->Children)
                    Pending.push_back(std::move(Grandchild));
                Child->Children.clear();
            }
        }
    }

    void Node::AppendChild(const std::shared_ptr<Node>& Child)
    {
        InsertBefore(Child, nullptr);
    }

    void Node::InsertBefore(const std::shared_ptr<Node>& ChildNode, const std::shared_ptr<Node>& ReferenceNode)
    {
        // Arguments may refer to entries in Children, which insertion can invalidate.
        const auto Child = ChildNode;
        const auto Reference = ReferenceNode;
        if (!Child)
            throw std::invalid_argument("Cannot insert a null child");
        if (Type != NodeType::Document && Type != NodeType::Element)
            throw std::invalid_argument("Only documents and elements can have children");
        if (Child->Type == NodeType::Document)
            throw std::invalid_argument("Cannot insert a document as a child");

        auto Self = shared_from_this();
        if (Child.get() == this)
            throw std::invalid_argument("Cannot insert a node into itself");
        // New nodes cannot be ancestors, keeping the parser's append path constant-time.
        if (!Child->Children.empty())
            for (auto Ancestor = Parent.lock(); Ancestor; Ancestor = Ancestor->Parent.lock())
                if (Ancestor == Child)
                    throw std::invalid_argument("Cannot insert an ancestor into its descendant");

        auto Position = Reference ? std::find(Children.begin(), Children.end(), Reference) : Children.end();
        if (Reference && Position == Children.end())
            throw std::invalid_argument("Reference is not a child of this node");
        if (Child == Reference)
            return;

        // Reserve before detaching so allocation failure leaves the tree intact.
        auto Target = static_cast<std::size_t>(Position - Children.begin());
        auto PreviousParent = Child->Parent.lock();
        if (Children.size() == Children.capacity())
            Children.reserve(Children.size() + std::max<std::size_t>(Children.size(), 1));
        if (PreviousParent)
        {
            if (PreviousParent.get() == this)
            {
                auto Previous = std::find(Children.begin(), Children.end(), Child);
                if (Previous != Children.end() && static_cast<std::size_t>(Previous - Children.begin()) < Target)
                    --Target;
            }
            PreviousParent->RemoveChild(Child);
        }
        Children.insert(Children.begin() + static_cast<std::ptrdiff_t>(Target), Child);
        Child->Parent = Self;
    }

    bool Node::RemoveChild(const std::shared_ptr<Node>& Child)
    {
        const auto Position = std::find(Children.begin(), Children.end(), Child);
        if (Position == Children.end())
            return false;
        if (*Position)
            (*Position)->Parent.reset();
        Children.erase(Position);
        return true;
    }

    void Node::Remove()
    {
        if (auto Owner = Parent.lock())
            Owner->RemoveChild(shared_from_this());
    }

    std::string Node::GetAttribute(const std::string& Name) const
    {
        const auto Position = Attributes.find(Utils::ToLower(Name));
        return Position != Attributes.end() ? Position->second : "";
    }

    bool Node::HasAttribute(const std::string& Name) const
    {
        return Attributes.find(Utils::ToLower(Name)) != Attributes.end();
    }

    void Node::SetAttribute(const std::string& Name, const std::string& Value)
    {
        Attributes[Utils::ToLower(Name)] = Value;
    }

    bool Node::RemoveAttribute(const std::string& Name)
    {
        return Attributes.erase(Utils::ToLower(Name)) != 0;
    }

    std::string Node::GetTextContent() const
    {
        if (Type == NodeType::Text || Type == NodeType::Comment || Type == NodeType::Doctype)
            return Text;
        std::string Result;
        std::vector<const Node*> Pending{this};
        while (!Pending.empty())
        {
            const auto* Current = Pending.back();
            Pending.pop_back();
            if (!Current)
                continue;
            if (Current->Type == NodeType::Text)
                Result += Current->Text;
            else if (Current->Type == NodeType::Element || Current->Type == NodeType::Document)
                for (auto It = Current->Children.rbegin(); It != Current->Children.rend(); ++It)
                    Pending.push_back(It->get());
        }
        return Result;
    }

    void Node::SetTextContent(const std::string& TextContent)
    {
        if (Type != NodeType::Element && Type != NodeType::Document)
        {
            Text = TextContent;
            return;
        }
        std::shared_ptr<Node> Replacement;
        if (!TextContent.empty())
        {
            Replacement = std::make_shared<Node>(NodeType::Text);
            Replacement->Text = TextContent;
            Replacement->Parent = shared_from_this();
        }
        for (const auto& Child : Children)
            if (Child)
                Child->Parent.reset();
        Children.clear();
        if (Replacement)
            Children.push_back(std::move(Replacement));
    }

    bool Node::HasClass(const std::string& ClassName) const
    {
        const auto Position = Attributes.find("class");
        return Position != Attributes.end() && Utils::ContainsWord(Position->second, ClassName);
    }

    std::vector<std::shared_ptr<Node>> Node::Select(std::string_view Pattern) const
    {
        return Query(std::const_pointer_cast<Node>(shared_from_this())).Select(Pattern);
    }

    std::shared_ptr<Node> Node::SelectFirst(std::string_view Pattern) const
    {
        return Query(std::const_pointer_cast<Node>(shared_from_this())).SelectFirst(Pattern);
    }

    bool Node::Matches(std::string_view Pattern) const
    {
        return Selector(Pattern).Matches(*this);
    }

    std::shared_ptr<Node> Node::Closest(std::string_view Pattern)
    {
        const Selector Compiled(Pattern);
        for (auto Current = shared_from_this(); Current; Current = Current->Parent.lock())
            if (Compiled.Matches(*Current, this))
                return Current;
        return nullptr;
    }

    std::string Node::ToHtml() const
    {
        return Detail::Serialize(*this);
    }
} // namespace HtmlParser
