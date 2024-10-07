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
        DOM(const std::shared_ptr<Node>& Root);

        std::shared_ptr<Node> Root() const;

        void Traverse(const std::function<void(const std::shared_ptr<Node>&)>& Visitor) const;

        std::vector<std::shared_ptr<Node>> GetElementsByTagName(const std::string& TagName) const;
        std::vector<std::shared_ptr<Node>> GetElementsByClassName(const std::string& ClassName) const;
        std::shared_ptr<Node> GetElementById(const std::string& Id) const;

        std::string ToHtml() const;

    private:
        std::shared_ptr<Node> Document;

        void TraverseImpl(const std::shared_ptr<Node>& ElementNode, const std::function<void(const std::shared_ptr<Node>&)>& Visitor) const;

        void GetElementsByTagNameImpl(const std::shared_ptr<Node>& ElementNode, const std::string& TagName, std::vector<std::shared_ptr<Node>>& Elements) const;

        void GetElementsByClassNameImpl(const std::shared_ptr<Node>& ElementNode, const std::string& ClassName, std::vector<std::shared_ptr<Node>>& Elements) const;

        void GetElementByIdImpl(const std::shared_ptr<Node>& ElementNode, const std::string& Id, std::shared_ptr<Node>& Result) const;

        void ToHtmlImpl(const std::shared_ptr<Node>& ElementNode, std::string& Html) const;
    };
} // namespace HtmlParser
