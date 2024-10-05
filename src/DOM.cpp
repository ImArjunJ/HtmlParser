#include <HtmlParser/DOM.hpp>
#include <HtmlParser/Query.hpp>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace HtmlParser
{
    const std::unordered_set<std::string> SelfClosingTags = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "source", "track", "wbr"};

    std::string HtmlParser::DOM::ToHtml() const
    {
        std::function<std::string(const std::shared_ptr<Node>&)> SerializeNode = [&](const std::shared_ptr<Node>& TargetNode) -> std::string
        {
            if (!TargetNode)
                return "";

            std::ostringstream HtmlStream;

            // Opening tag
            HtmlStream << "<" << TargetNode->Tag;
            for (const auto& [attr_name, attr_value] : TargetNode->Attributes)
            {
                HtmlStream << " " << attr_name << "=\"" << attr_value << "\"";
            }

            // Check if the tag is a known self-closing tag
            if (SelfClosingTags.find(TargetNode->Tag) != SelfClosingTags.end())
            {
                HtmlStream << " />"; // Self-closing tag
            }
            else
            {
                HtmlStream << ">";

                // Text content
                if (!TargetNode->Text.empty())
                {
                    HtmlStream << TargetNode->Text;
                }

                // Recursively serialize children
                for (const auto& child : TargetNode->Children)
                {
                    HtmlStream << SerializeNode(child);
                }

                // Closing tag
                HtmlStream << "</" << TargetNode->Tag << ">";
            }

            return HtmlStream.str();
        };

        // Start with the root node and serialize the entire DOM
        return SerializeNode(m_Root);
    }

    // Find the first node matching a CSS selector using the Query class
    std::shared_ptr<Node> DOM::Find(const std::string& selector) const
    {
        Query Query;
        auto Elements = Query.FindAll(*this, selector);
        return Elements.empty() ? nullptr : Elements.front();
    }

    // Find all nodes matching a CSS selector using the Query class
    std::vector<std::shared_ptr<Node>> DOM::FindAll(const std::string& selector) const
    {
        Query Query;
        return Query.FindAll(*this, selector);
    }

    std::vector<std::shared_ptr<Node>> DOM::GetRoots() const
    {
        return m_Root ? m_Root->Children : std::vector<std::shared_ptr<Node>>();
    }
} // namespace HtmlParser
