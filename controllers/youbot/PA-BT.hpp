#pragma once
#include "YouBot.hpp"
#include <algorithm>
#include <unordered_map>

using namespace YouBot;

namespace PABT
{
    std::unordered_map<Node*, std::vector<Node*>> preConditions;
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
        Node* dropBoxOnTray = new Action<>(unGrip);
        Node* dropBallOnTray = new Action<>(unGrip);
        Node* setHeight4 = new Action<int>(setHeight, 4);
        
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
        Node* isHeight4 = new Condition<int>(isHeight, 4);
        Node* boxIsntBlocked = new Condition<Object>(isntBlocked, box);
        Node* ballIsntBlocked = new Condition<Object>(isntBlocked, ball);
        Node* isEmptyForPick = new Condition<>(isEmpty);

        postConditions[faceBox] = isFacingBox;
        postConditions[faceBall] = isFacingBall;
        postConditions[faceGoal] = isFacingGoal;

        preConditions[approachBox] = {boxIsntBlocked, isFacingBox};
        postConditions[approachBox] = isAtBox;
        preConditions[approachBall] = {ballIsntBlocked, isFacingBall};
        postConditions[approachBall] = isAtBall;
        preConditions[approachGoal] = {isFacingGoal};
        postConditions[approachGoal] = isAtGoal;

        preConditions[liftUpBox] = {isAtBox, boxIsGrabbed};
        postConditions[liftUpBox] = boxIsHeld;
        preConditions[liftUpBall] = {isAtBall, ballIsGrabbed};
        postConditions[liftUpBall] = ballIsHeld;

        preConditions[grabBox] = {isBowedToBox};
        postConditions[grabBox] = boxIsGrabbed;
        preConditions[grabBall] = {isBowedToBall};
        postConditions[grabBall] = ballIsGrabbed;

        preConditions[bowToBox] = {isUnGrippedForPick, isGrippedAroundBox};
        postConditions[bowToBox] = isBowedToBox;
        preConditions[bowToBall] = {isUnGrippedForPick, isGrippedAroundBall};
        postConditions[bowToBall] = isBowedToBall;

        preConditions[unGripForPick] = {isEmptyForPick};
        postConditions[unGripForPick] = isUnGrippedForPick;
        postConditions[gripAroundBox] = isGrippedAroundBox;
        postConditions[gripAroundBall] = isGrippedAroundBall;

        preConditions[dropBoxOnGoal] = {boxIsHeld, isAtGoal, isHeight1};
        postConditions[dropBoxOnGoal] = boxIsOnGoal;
        preConditions[dropBallOnGoal] = {ballIsHeld, isAtGoal, isHeight1};
        postConditions[dropBallOnGoal] = ballIsOnGoal;

        postConditions[setHeight1] = isHeight1;

        preConditions[passByBox] = {isFacingAwayBox};
        postConditions[passByBox] = ballIsntBlocked;
        preConditions[passByBall] = {isFacingAwayBall};
        postConditions[passByBall] = boxIsntBlocked;

        postConditions[faceAwayBox] = isFacingAwayBox;
        postConditions[faceAwayBall] = isFacingAwayBall;

        preConditions[dropBoxOnTray] = {boxIsHeld, isHeight4};
        postConditions[dropBoxOnTray] = ballIsntBlocked;
        preConditions[dropBallOnTray] = {ballIsHeld, isHeight4};
        postConditions[dropBallOnTray] = boxIsntBlocked;

        postConditions[setHeight4] = isHeight4;

        if (obj == box)
            return new Sequence({boxIsOnGoal});
        else if (obj == ball)
            return new Sequence({ballIsOnGoal});
        else
            return new Sequence({boxIsOnGoal, ballIsOnGoal});
    }

    Node* replan(Node* root)
    {
        Node*& condition = root -> getFailure();
        std::vector<Node*> actions, temp;
        Node* fallback = new Fallback({condition}),* sequence;

        for (auto pair : postConditions)
            if (pair.second == condition)
                actions.push_back(pair.first);

        if (actions.empty())
            return nullptr;

        for (Node* action : actions)
        {
            sequence = new Sequence(preConditions[action]);

            temp = sequence -> getChildren();
            temp.push_back(action);
            ((Sequence*) sequence) -> setChildren(temp);

            temp = fallback -> getChildren();
            temp.push_back(sequence);
            ((Fallback*) fallback) -> setChildren(temp);
        }

        condition = fallback;

        return root;
    }
}