#pragma once
#include "YouBot.hpp"
#include <fstream>

using namespace YouBot;

namespace Manual
{
    std::vector<Node*> parseXML(std::vector<std::string>& lines, int start, int end, Object obj) 
    {
        std::vector<Node*> nodes;
        int count = 0;
        std::string nodeType;

        for (int i = start; i <= end; i++)
        {
            if (!count)
            {
                if (lines[i][0] == '<')
                {
                    nodeType = lines[i].substr(1, lines[i].find('>') - 1);
                    start = i;
                    count++;
                }
                else if (lines[i][0] == '[' || lines[i][0] == '(')
                {
                    Object arg;

                    if (lines[i].find("goal") != -1)
                        arg = goal;
                    else if (lines[i].find("box") != -1)
                        arg = box;
                    else if (lines[i].find("ball") != -1)
                        arg = ball;
                    else if (lines[i].find("obj") != -1)
                        arg = obj;
                    else if (lines[i].find("other") != -1)
                    {
                        if (obj == box)
                            arg = ball;
                        else
                            arg = box;
                    }

                    if (lines[i].find("isAt") != -1)
                        nodes.push_back(new Condition<Object>(isAt, arg));
                    else if (lines[i].find("isHeld") != -1)
                        nodes.push_back(new Condition<Object>(isHeld, arg));
                    else if (lines[i].find("isOnGoal") != -1)
                        nodes.push_back(new Condition<Object>(isOnGoal, arg));
                    else if (lines[i].find("isFacing") != -1)
                        nodes.push_back(new Condition<Object>(isFacing, arg));
                    else if (lines[i].find("isHeight") != -1)
                        nodes.push_back(new Condition<int>(isHeight, lines[i][lines[i].find(' ') + 1] - '0'));
                    else if (lines[i].find("isGrabbed") != -1)
                        nodes.push_back(new Condition<Object>(isGrabbed, arg));
                    else if (lines[i].find("isUnGripped") != -1)
                        nodes.push_back(new Condition<>(isUnGripped));
                    else if (lines[i].find("isGrippedAround") != -1)
                        nodes.push_back(new Condition<Object>(isGrippedAround, arg));
                    else if (lines[i].find("isFacingAway") != -1)
                        nodes.push_back(new Condition<Object>(isFacingAway, arg));
                    else if (lines[i].find("isntBlocked") != -1)
                        nodes.push_back(new Condition<Object>(isntBlocked, arg));
                    else if (lines[i].find("face") != -1)
                        nodes.push_back(new Action<Object>(face, arg));
                    else if (lines[i].find("approach") != -1)
                        nodes.push_back(new Action<Object>(approach, arg));
                    else if (lines[i].find("setHeight") != -1)
                        nodes.push_back(new Action<int>(setHeight, lines[i][lines[i].find(' ') + 1] - '0'));
                    else if (lines[i].find("grab") != -1)
                        nodes.push_back(new Action<Object>(grab, arg));
                    else if (lines[i].find("unGrip") != -1)
                        nodes.push_back(new Action<>(unGrip));
                    else if (lines[i].find("gripAround") != -1)
                        nodes.push_back(new Action<Object>(gripAround, arg));
                    else if (lines[i].find("faceAway") != -1)
                        nodes.push_back(new Action<Object>(faceAway, arg));
                }
            }
            else if (lines[i][0] == '<')
            {
                if (lines[i][1] != '/')
                    count++;
                else
                    count--;

                if (!count)
                {
                    std::vector<Node*> children = parseXML(lines, start + 1, i - 1, obj);

                    if (nodeType == "sequence")
                        nodes.push_back(new Sequence(children));
                    else if (nodeType == "fallback")
                        nodes.push_back(new Fallback(children));
                    else if (nodeType == "parallel")
                        nodes.push_back(new Parallel(children));
                    else if (nodeType == "decorator")
                        nodes.push_back(new Decorator(children[0]));
                    else if (nodeType == "sequence*")
                        nodes.push_back(new SequenceStar(children));
                    else if (nodeType == "fallback*")
                        nodes.push_back(new FallbackStar(children));
                    else if (nodeType.find("percentSuccess") != -1)
                        nodes.push_back(new PercentSuccess(children[0], std::stoi(nodeType.substr(nodeType.find(' ') + 1))));
                }
            }
        }

        if (count)
            std::cout << "unbalanced opening / closing tags" << std::endl;

        return nodes;
    }

    Node* parseXML(std::string name, Object obj)
    {
        std::vector<std::string> lines;
        std::string line;
        std::ifstream file(name);

        while (getline(file, line))
        {
            while (line[0] != '<' && line[0] != '[' && line[0] != '(')
                line.erase(0, 1);
            
            if (!line.empty())
                lines.push_back(line);
        }

        file.close();

        return parseXML(lines, 0, lines.size() - 1, obj)[0];
    }
}