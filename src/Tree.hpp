#pragma once

#include <HtmlParser/Node.hpp>

#include <algorithm>
#include <string_view>

namespace HtmlParser::Detail
{
    // Read-only traversal uses borrowed pointers and O(depth) workspace.
    template <typename NodePointer, typename Visitor> void Walk(NodePointer Root, Visitor&& Visit)
    {
        if (!Root || !Visit(Root))
            return;
        struct Frame
        {
            NodePointer Current;
            std::size_t NextChild;
        };
        std::vector<Frame> Path{{Root, 0}};
        while (!Path.empty())
        {
            auto& Frame = Path.back();
            if (Frame.NextChild == Frame.Current->Children.size())
            {
                Path.pop_back();
                continue;
            }
            NodePointer Child = Frame.Current->Children[Frame.NextChild++].get();
            if (!Child)
                continue;
            if (!Visit(Child))
                return;
            if (!Child->Children.empty())
                Path.push_back({Child, 0});
        }
    }

    std::string Serialize(const Node& Root);
} // namespace HtmlParser::Detail
