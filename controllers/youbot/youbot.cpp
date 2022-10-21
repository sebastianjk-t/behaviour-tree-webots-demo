#include "Manual.hpp"
#include "IPA-BT.hpp"
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

    srand(time(nullptr));

    bool usePlan = string(input[1]).empty(), drop = false;
    Object obj = (string(input[2]) == "ball") ? ball : box;

    Node* root;

    if (usePlan)
        root = PABT::plan(obj);
    else
        root = Manual::parseXML("xmls/" + string(input[1]) + ".xml", obj);

    int ticks, prevCount = -1, successes = 0;
    double start = wb_robot_get_time();

    for (ticks = 0; successes < 10 && wb_robot_step(timeStep) != -1; ticks++)
    {
        switch (root -> tick())
        {
            case SUCCESS: 

                if (string(input[1]) == "game")
                {
                    cout << "successes: " << ++successes << endl;
                    ((FallbackStar*) root) -> reset();
                }
                else
                {
                    cout << ((string(input[1]).empty()) ? "pa-bt" : input[1]) << " - " << count(root) << " nodes (success!)" << endl;
                    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
                    return deinit();
                }
                
                break;

            case FAILURE: 

                if (usePlan)
                    root = PABT::replan(root);

                if (string(input[1]) != "game" && (!usePlan || !root))
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
                    const double pos[3] = {0, 0, 0.1}, rot[4] = {0, 0, 1, 0};
                    wb_supervisor_field_set_sf_vec3f(obj.trans, pos);
                    wb_supervisor_field_set_sf_rotation(obj.rot, rot);
                    wb_supervisor_node_reset_physics(obj.node);
                    drop = false;
                }
        }

        if (string(input[1]) == "game")
        {
            if (isOnGoal(box))
            {
                const double pos[3] = {0, 0.5, 0.1}, rot[4] = {0, 0, 1, 0};
                wb_supervisor_field_set_sf_vec3f(box.trans, pos);
                wb_supervisor_field_set_sf_rotation(box.rot, rot);
                wb_supervisor_node_reset_physics(box.node);
            }
            else if (isOnGoal(ball))
            {
                const double pos[3] = {1.0, 1.0, 0.1}, rot[4] = {0, 0, 1, 0};
                wb_supervisor_field_set_sf_vec3f(ball.trans, pos);
                wb_supervisor_field_set_sf_rotation(ball.rot, rot);
                wb_supervisor_node_reset_physics(ball.node);
            }
        }
    }

    cout << ((string(input[1]).empty()) ? "pa-bt" : input[1]) << " - " << count(root) << " nodes (complete?)" << endl;
    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;

    return deinit();
}