#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "Node.hpp"

namespace HtmlParser
{
    class DOM
    {
    public:
        DOM();
        DOM(const std::shared_ptr<Node>& Root);

        std::shared_ptr<Node> Root() const;

        void Traverse(const std::function<void(const std::shared_ptr<Node>&)>& Visitor) const;
        std::vector<std::shared_ptr<Node>> GetElementsByTagName(const std::string& TagName) const;
        std::vector<std::shared_ptr<Node>> GetElementsByClassName(const std::string& ClassName) const;
        std::shared_ptr<Node> GetElementById(const std::string& Id) const;

        std::string ToHtml() const;
        std::vector<std::shared_ptr<Node>> Select(std::string_view Pattern) const;
        std::shared_ptr<Node> SelectFirst(std::string_view Pattern) const;

    private:
        std::shared_ptr<Node> m_Document;
    };
} // namespace HtmlParser
