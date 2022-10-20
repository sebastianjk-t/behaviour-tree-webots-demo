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
    bool drop = true;

    Node* root;

    if (usePlan)
        root = PABT::plan(obj);
    else
        root = Manual::parseXML("xmls/" + string(input[1]) + ".xml", obj);

    int prevCount = -1;
    double start = wb_robot_get_time();

    for (int ticks = 0; wb_robot_step(timeStep) != -1; ticks++)
    {
        switch (root -> tick())
        {
            case SUCCESS: 

                cout << ((string(input[1]).empty()) ? "pa-bt" : input[1]) << " - " << count(root) << " nodes (success!)" << endl;
                cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
                return deinit();

            case FAILURE: 

                cout << count(root) << endl;

                if (usePlan)
                    root = PABT::replan(root);

                cout << count(root) << endl;
                    
                if (!usePlan || !root)
                {
                    cout << ((string(input[1]).empty()) ? "pa-bt" : input[1]) << " - " << count(root) << " nodes (failure :()" << endl;
                    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
                    return deinit();
                }

                break;

            case RUNNING:

                if (usePlan && count(root) != prevCount)
                {
                    prevCount = count(root);
                    cout << ((string(input[1]).empty()) ? "pa-bt" : input[1]) << " - " << count(root) << " nodes (running...)" << endl;
                    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
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