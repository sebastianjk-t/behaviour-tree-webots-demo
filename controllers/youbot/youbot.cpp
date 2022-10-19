#include "Manual.hpp"
#include "PA-BT.hpp"
#include <iostream>

using namespace std;

int count(Node* node)
{
    if (!node)
        return 0;

    int counter = 1;

    for (Node* child : node -> getChildren())
        counter += count(child);

    return counter;
}

int main(int n, char** input)
{
    if (init())
        return 1;

    bool usePlan = string(input[1]).empty();
    Object obj = (string(input[2]) == "ball") ? ball : box;
    bool drop = false;

    Node* root;

    if (usePlan)
        root = PABT::plan(obj);
    else
        root = Manual::parseXML("xmls/" + string(input[1]) + ".xml", obj);

    double start = wb_robot_get_time();
    int prevCount = -1;

    while (wb_robot_step(timeStep) != -1)
    {
        switch (root -> tick())
        {
            case SUCCESS: 

                cout << wb_robot_get_time() - start << " seconds - " << count(root) << " nodes (success!)" << endl;
                return deinit();

            case FAILURE: 

                cout << count(root) << endl;

                if (usePlan)
                    root = PABT::replan(root);

                cout << count(root) << endl;
                    
                if (!usePlan || !root)
                {
                    cout << wb_robot_get_time() - start << " seconds - " << count(root) << " nodes (failure...)" << endl;
                    return deinit();
                }

                break;

            case RUNNING:

                if (usePlan && count(root) != prevCount)
                {
                    prevCount = count(root);
                    cout << wb_robot_get_time() - start << " seconds - " << prevCount << " nodes" << endl;
                }

                if (drop && isFacing(goal))
                {
                    const double pos[3] = {0, 0, 0.25};
                    wb_supervisor_field_set_sf_vec3f(obj.trans, pos);
                    wb_supervisor_node_reset_physics(obj.node);
                    drop = false;
                }
        }
    }

    return deinit();
}