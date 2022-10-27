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
    if (n != 3 || init())
        return 1;

    srand(time(nullptr));

    bool drop = false, pabt = false, ipabt = false, game = false;
    Object obj = ball;
    const double pos[3] = {-2, 0, 0.02}, rot[4] = {0, 0, 1, 0};

    if (string(input[1]) == "easy")
    {
        obj = box;
        wb_supervisor_field_set_sf_vec3f(ball.trans, pos);
        wb_supervisor_field_set_sf_rotation(ball.rot, rot);
        wb_supervisor_node_reset_physics(ball.node);
    }
    else if (string(input[1]) == "medium")
    {
        drop = true;
        wb_supervisor_field_set_sf_vec3f(box.trans, pos);
        wb_supervisor_field_set_sf_rotation(box.rot, rot);
        wb_supervisor_node_reset_physics(box.node);
    }
    
    if (string(input[1]) == "game")
    {
        game = true;
        const double pos[3] = {-1, -1, 0.02};
        wb_supervisor_field_set_sf_vec3f(box.trans, pos);
        wb_supervisor_field_set_sf_rotation(box.rot, rot);
        wb_supervisor_node_reset_physics(box.node);
    }
    else if (string(input[2]) == "pabt" || string(input[2]) == "pa-bt")
        pabt = true;
    else if (string(input[2]) == "ipabt" || string(input[2]) == "ipa-bt")
        ipabt = true;

    Node* root;

    if (pabt || ipabt)
        root = PABT::plan(obj);
    else
        root = Manual::parseXML("xmls/" + string(input[2]) + ".xml", obj);

    //root -> getChildren()[1] -> increaseUtility();

    int ticks, prevCount = -1, successes = 0;
    double start = wb_robot_get_time();

    for (ticks = 0; wb_robot_get_time() < 600.0 && wb_robot_step(timeStep) != -1; ticks++)
    {
        switch (root -> tick())
        {
            case SUCCESS: 

                if (game)
                {
                    cout << "successes: " << ++successes << endl;
                    root -> getChildren()[((FallbackStar*) root) -> getIndex()] -> increaseUtility();
                    ((FallbackStar*) root) -> reset();
                }
                else
                {
                    cout << input[1] << " - " << input[2] << " - " << count(root) << " nodes (success!!)" << endl;
                    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
                    return deinit();
                }
                
                break;

            case FAILURE: 

                if (pabt)
                    root = PABT::replan(root);
                else if (ipabt)
                    root = IPABT::replan(root);

                if (!game && ((!pabt && !ipabt) || !root))
                {
                    cout << input[1] << " - " << input[2] << " - " << count(root) << " nodes (failure :()" << endl;
                    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
                    return deinit();
                }

                break;

            case RUNNING:

                if ((pabt || ipabt) && count(root) != prevCount)
                {
                    prevCount = count(root);
                    cout << input[1] << " - " << input[2] << " - " << count(root) << " nodes (running...)" << endl;
                    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
                }

                if (drop && isFacing(goal))
                {
                    const double pos[3] = {0, 0, 0.02};
                    wb_supervisor_field_set_sf_vec3f(obj.trans, pos);
                    wb_supervisor_field_set_sf_rotation(obj.rot, rot);
                    wb_supervisor_node_reset_physics(obj.node);
                    drop = false;
                }
        }

        if (game)
        {
            if (isOnGoal(box))
            {
                const double pos[3] = {-1, -1, 0.02};

                wb_supervisor_field_set_sf_vec3f(box.trans, pos);
                wb_supervisor_field_set_sf_rotation(box.rot, rot);
                wb_supervisor_node_reset_physics(box.node);

                if (((FallbackStar*) root) -> getIndex())
                    root -> getChildren()[((FallbackStar*) root) -> getIndex() - 1] -> increaseUtility(-1);
            }
            else if (isOnGoal(ball))
            {
                const double pos[3] = {1, 1, 0.02};

                wb_supervisor_field_set_sf_vec3f(ball.trans, pos);
                wb_supervisor_field_set_sf_rotation(ball.rot, rot);
                wb_supervisor_node_reset_physics(ball.node);

                if (((FallbackStar*) root) -> getIndex())
                    root -> getChildren()[((FallbackStar*) root) -> getIndex() - 1] -> increaseUtility(-1);
            }
        }
    }

    cout << input[1] << " - " << input[2] << " - " << count(root) << " nodes (complete?)" << endl;
    cout << wb_robot_get_time() - start << " seconds - " << ticks << " ticks" << endl;
    cout << "total successes - " << successes << endl;

    return deinit();
}