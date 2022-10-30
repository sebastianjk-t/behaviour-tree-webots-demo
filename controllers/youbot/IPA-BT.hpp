#pragma once
#include "PA-BT.hpp"

using namespace PABT;

namespace IPABT
{
    Node* replan(Node* root)
    {
        Node*& condition = (root -> getChildren().empty()) ? root : root -> getFailure();
        std::vector<Node*> actions, temp, fallback{condition};

        for (auto pair : postConditions)
            if (pair.second == condition)
                actions.push_back(pair.first);

        if (actions.empty())
            return nullptr;

        if (actions.size() == 1 && preConditions[actions.front()].empty())
        {
            condition = actions.front();
            return root;
        }

        for (Node* action : actions)
        {
            if (preConditions[action].empty())
            {
                fallback.push_back(action);
                continue;
            }

            temp.clear();

            for (std::vector<Node*> group : preConditions[action])
            {
                if (group.size() == 1)
                    temp.push_back(group[0]);
                else
                    temp.push_back(new Parallel(group));
            }

            temp.push_back(action);
            fallback.push_back(new Sequence(temp));
        }

        condition = new Fallback(fallback);
        return root;
    }
}