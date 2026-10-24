#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Node.hpp"

namespace HtmlParser
{
    class Selector
    {
    public:
        explicit Selector(std::string_view Pattern);
        ~Selector();
        Selector(const Selector& Other);
        Selector& operator=(const Selector& Other);
        Selector(Selector&&) noexcept;
        Selector& operator=(Selector&&) noexcept;

        bool IsValid() const noexcept;
        std::string_view Error() const noexcept;
        bool Matches(const Node& Element, const Node* Scope = nullptr) const;

    private:
        friend class Query;
        struct Impl;
        std::unique_ptr<Impl> m_Impl;
    };

    class Query
    {
    public:
        explicit Query(const std::shared_ptr<Node>& QueryRoot);
        std::vector<std::shared_ptr<Node>> Select(std::string_view Pattern) const;
        std::vector<std::shared_ptr<Node>> Select(const Selector& Pattern) const;
        std::shared_ptr<Node> SelectFirst(std::string_view Pattern) const;
        std::shared_ptr<Node> SelectFirst(const Selector& Pattern) const;

    private:
        std::shared_ptr<Node> m_Root;
    };
} // namespace HtmlParser
