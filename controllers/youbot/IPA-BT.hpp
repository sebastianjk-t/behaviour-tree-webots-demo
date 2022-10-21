#pragma once
#include "PA-BT.hpp"

using namespace PABT;

namespace IPABT
{
    Node* replan(Node* root)
    {
        Node*& condition = (root -> getChildren().empty()) ? root : root -> getFailure();
        std::vector<Node*> actions, temp;
        Node* fallback = new Fallback({condition}),* sequence;

        for (auto pair : postConditions)
            if (pair.second == condition)
                actions.push_back(pair.first);

        if (actions.empty())
            return nullptr;

        for (Node* action : actions)
        {
            if (preConditions[action].empty())
            {
                if (actions.size() == 1)
                {
                    condition = action;
                    return root;
                }
                
                temp = fallback -> getChildren();
                temp.push_back(action);
                ((Fallback*) fallback) -> setChildren(temp);
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
            sequence = new Sequence(temp);

            temp = fallback -> getChildren();
            temp.push_back(sequence);
            ((Fallback*) fallback) -> setChildren(temp);
        }

        condition = fallback;

        return root;
    }
}