#pragma once

#include <HtmlParser/Node.hpp>

#include <algorithm>
#include <string_view>

namespace HtmlParser::Detail
{
    template <typename Visitor>
    void Walk(Node* Root, Visitor&& Visit)
    {
        if (!Root)
            return;
        std::vector<Node*> Pending{Root};
        while (!Pending.empty())
        {
            auto* Current = Pending.back();
            Pending.pop_back();
            if (!Current)
                continue;
            if (!Visit(Current))
                return;
            for (auto It = Current->Children.rbegin(); It != Current->Children.rend(); ++It)
                Pending.push_back(It->get());
        }
    }

    std::string Serialize(const Node& Root);
} // namespace HtmlParser::Detail
