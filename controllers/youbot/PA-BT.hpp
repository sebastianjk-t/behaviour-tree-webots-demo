#pragma once
#include "YouBot.hpp"
#include <unordered_map>

using namespace YouBot;

namespace PABT
{
    std::unordered_map<Node*, std::vector<std::vector<Node*>>> preConditions;
    std::unordered_map<Node*, Node*> postConditions;

    Node* plan(Object obj)
    {
        Node* faceBox = new Action<Object>(face, box);
        Node* faceBall = new Action<Object>(face, ball);
        Node* faceGoal = new Action<Object>(face, goal);
        Node* approachBox = new Action<Object>(approach, box);
        Node* approachBall = new Action<Object>(approach, ball);
        Node* approachGoal = new Action<Object>(approach, goal);
        Node* dropBoxOnGoal = new Action<>(unGrip);
        Node* dropBallOnGoal = new Action<>(unGrip);
        Node* setHeight1 = new Action<int>(setHeight, 1);
        Node* liftUpBox = new Action<int>(setHeight, 1);
        Node* liftUpBall = new Action<int>(setHeight, 1);
        Node* grabBox = new Action<Object>(grab, box);
        Node* grabBall = new Action<Object>(grab, ball);
        Node* bowToBox = new Action<int>(setHeight, 0);
        Node* bowToBall = new Action<int>(setHeight, 0);
        Node* unGripForPick = new Action<>(unGrip);
        Node* gripAroundBox = new Action<Object>(gripAround, box);
        Node* gripAroundBall = new Action<Object>(gripAround, ball);
        Node* passByBox = new Action<Object>(approach, ball);
        Node* passByBall = new Action<Object>(approach, box);
        Node* faceAwayBox = new Action<Object>(faceAway, box);
        Node* faceAwayBall = new Action<Object>(faceAway, ball);
        
        Node* isAtBox = new Condition<Object>(isAt, box);
        Node* isAtBall = new Condition<Object>(isAt, ball);
        Node* isAtGoal = new Condition<Object>(isAt, goal);
        Node* boxIsHeld = new Condition<Object>(isHeld, box);
        Node* ballIsHeld = new Condition<Object>(isHeld, ball);
        Node* boxIsOnGoal = new Condition<Object>(isOnGoal, box);
        Node* ballIsOnGoal = new Condition<Object>(isOnGoal, ball);
        Node* isFacingBox = new Condition<Object>(isFacing, box);
        Node* isFacingBall = new Condition<Object>(isFacing, ball);
        Node* isFacingGoal = new Condition<Object>(isFacing, goal);
        Node* isHeight1 = new Condition<int>(isHeight, 1);
        Node* boxIsGrabbed = new Condition<Object>(isGrabbed, box);
        Node* ballIsGrabbed = new Condition<Object>(isGrabbed, ball);
        Node* isBowedToBox = new Condition<int>(isHeight, 0);
        Node* isBowedToBall = new Condition<int>(isHeight, 0);
        Node* isUnGrippedForPick = new Condition<>(isUnGripped);
        Node* isGrippedAroundBox = new Condition<Object>(isGrippedAround, box);
        Node* isGrippedAroundBall = new Condition<Object>(isGrippedAround, ball);
        Node* isFacingAwayBox = new Condition<Object>(isFacingAway, box);
        Node* isFacingAwayBall = new Condition<Object>(isFacingAway, ball);
        Node* boxIsntBlocked = new Condition<Object>(isntBlocked, box);
        Node* ballIsntBlocked = new Condition<Object>(isntBlocked, ball);

        postConditions[faceBox] = isFacingBox;
        postConditions[faceBall] = isFacingBall;
        postConditions[faceGoal] = isFacingGoal;

        preConditions[approachBox] = {{boxIsntBlocked}, {isFacingBox}};
        postConditions[approachBox] = isAtBox;
        preConditions[approachBall] = {{ballIsntBlocked}, {isFacingBall}};
        postConditions[approachBall] = isAtBall;
        preConditions[approachGoal] = {{isFacingGoal}};
        postConditions[approachGoal] = isAtGoal;

        preConditions[liftUpBox] = {{isBowedToBox}, {boxIsGrabbed}};
        postConditions[liftUpBox] = boxIsHeld;
        preConditions[liftUpBall] = {{isBowedToBall}, {ballIsGrabbed}};
        postConditions[liftUpBall] = ballIsHeld;

        postConditions[grabBox] = boxIsGrabbed;
        postConditions[grabBall] = ballIsGrabbed;

        preConditions[bowToBox] = {{isAtBox, isUnGrippedForPick, isGrippedAroundBox}};
        postConditions[bowToBox] = isBowedToBox;
        preConditions[bowToBall] = {{isAtBall, isUnGrippedForPick, isGrippedAroundBall}};
        postConditions[bowToBall] = isBowedToBall;

        postConditions[unGripForPick] = isUnGrippedForPick;
        postConditions[gripAroundBox] = isGrippedAroundBox;
        postConditions[gripAroundBall] = isGrippedAroundBall;

        preConditions[dropBoxOnGoal] = {{boxIsHeld}, {isAtGoal}, {isHeight1}};
        postConditions[dropBoxOnGoal] = boxIsOnGoal;
        preConditions[dropBallOnGoal] = {{ballIsHeld}, {isAtGoal}, {isHeight1}};
        postConditions[dropBallOnGoal] = ballIsOnGoal;

        postConditions[setHeight1] = isHeight1;

        preConditions[passByBox] = {{isFacingAwayBox}};
        postConditions[passByBox] = ballIsntBlocked;
        preConditions[passByBall] = {{isFacingAwayBall}};
        postConditions[passByBall] = boxIsntBlocked;

        postConditions[faceAwayBox] = isFacingAwayBox;
        postConditions[faceAwayBall] = isFacingAwayBall;

        if (obj == box)
            return boxIsOnGoal;
        else if (obj == ball)
            return ballIsOnGoal;
        
        return new Fallback({new PercentSuccess(boxIsOnGoal, 40), new PercentSuccess(ballIsOnGoal, 60)}); // smart swaps first tick
    }

    Node* replan(Node*& root)
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
            temp.clear();

            for (std::vector<Node*> group : preConditions[action])
                for (Node* cond : group)
                    temp.push_back(cond);

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