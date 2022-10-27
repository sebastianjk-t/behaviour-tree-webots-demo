#pragma once

#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/position_sensor.h>
#include <webots/supervisor.h>

#include <cmath>
#include <vector>
#include <array>
#include <string>
#include <iostream>

#include "bts/Basic.hpp"
#include "bts/Smart.hpp"
#include "bts/Utility.hpp"

using namespace Utility;

struct Object
{
    WbNodeRef node;
    WbFieldRef trans;
    WbFieldRef rot;

    bool operator==(Object& b)
    {
        return trans == b.trans;
    }
    
    bool operator!=(Object& b)
    {
        return trans != b.trans;
    }
};

struct Device
{
    WbDeviceTag motor;
    WbDeviceTag sensor;
};

namespace YouBot
{
    int timeStep;

    Object robot, goal, box, ball;
    std::vector<Object> objects; // for iterating over objects (e.g. collision)

    std::array<Device, 4> wheels;
    std::array<Device, 5> arm;
    std::array<Device, 2> gripper;

    std::array<std::array<double, 3>, 7> heights;

    bool init()
    {
        wb_robot_init();

        timeStep = wb_robot_get_basic_time_step();

        robot.node = wb_supervisor_node_get_self();
        goal.node = wb_supervisor_node_get_from_def("goal");
        box.node = wb_supervisor_node_get_from_def("box");
        ball.node = wb_supervisor_node_get_from_def("ball");

        robot = {robot.node, wb_supervisor_node_get_field(robot.node, "translation"), wb_supervisor_node_get_field(robot.node, "rotation")};
        goal = {goal.node, wb_supervisor_node_get_field(goal.node, "translation"), wb_supervisor_node_get_field(goal.node, "rotation")};
        box = {box.node, wb_supervisor_node_get_field(box.node, "translation"), wb_supervisor_node_get_field(box.node, "rotation")};
        ball = {ball.node, wb_supervisor_node_get_field(ball.node, "translation"), wb_supervisor_node_get_field(ball.node, "rotation")};

        objects.push_back(box);
        objects.push_back(ball);

        for (int i = 0; i < 4; i++)
            wheels[i] = {wb_robot_get_device(("wheel" + std::to_string(i + 1)).c_str()), wb_robot_get_device(("wheel" + std::to_string(i + 1) + "sensor").c_str())};

        for (int i = 0; i < 5; i++)
            arm[i] = {wb_robot_get_device(("arm" + std::to_string(i + 1)).c_str()), wb_robot_get_device(("arm" + std::to_string(i + 1) + "sensor").c_str())};

        for (int i = 0; i < 2; i++)
            gripper[i] = {wb_robot_get_device(("finger" + std::to_string(i + 1)).c_str()), wb_robot_get_device(("finger" + std::to_string(i + 1) + "sensor").c_str())};

        for (Device& w : wheels)
            wb_position_sensor_enable(w.sensor, timeStep);

        for (Device& a : arm)
            wb_position_sensor_enable(a.sensor, timeStep);

        for (Device& g : gripper)
            wb_position_sensor_enable(g.sensor, timeStep);

        heights[0] = {-0.97, -1.55, -0.61};
        heights[1] = {-0.62, -0.98, -1.53};
        heights[2] = {0, -0.77, -1.21};
        heights[3] = {1.57, -2.635, 1.78};
        heights[4] = {0.678, 0.682, 1.74};
        heights[5] = {0.92, 0.42, 1.78};
        heights[6] = {-0.4, -1.2, -M_PI_2};

        for (int i = 0; i < 3; i++)
            wb_motor_set_position(arm[i + 1].motor, heights[2][i]);

        return 0;
    }

    bool deinit()
    {
        wb_robot_cleanup();
        return 0;
    }

    bool areSame(double a, double b, double e = 0.001)
    {
        return fabs(a - b) < e;
    }

// wheels

    void moveForward(double speed)
    {
        for (Device& w : wheels)
            wb_motor_set_position(w.motor, wb_position_sensor_get_value(w.sensor) + speed);
    }

    void turnRight(double speed)
    {
        for (int i = 0; i < wheels.size(); i++)
            wb_motor_set_position(wheels[i].motor, wb_position_sensor_get_value(wheels[i].sensor) - speed + speed * 2 * (i % 2));
    }

// arm

    double getArmRot()
    {
        return wb_position_sensor_get_value(arm[0].sensor);
    }

    bool isArmRot(double angle)
    {
        return areSame(getArmRot(), angle);
    }
    
    Status setArmRotation(int angle)
    {
        if (isArmRot(angle))
            return SUCCESS;

        if (abs(angle) > 2.95)
            return FAILURE;

        wb_motor_set_position(arm[0].motor, angle);
        return RUNNING;
    }

    bool isHeight(int height)
    {
        return areSame(wb_position_sensor_get_value(arm[1].sensor), heights[height][0]) && areSame(wb_position_sensor_get_value(arm[2].sensor), heights[height][1]) && areSame(wb_position_sensor_get_value(arm[3].sensor), heights[height][2]);
    }

    Status setHeight(int height)
    {
        if (isHeight(height))
            return SUCCESS;

        wb_motor_set_position(arm[1].motor, heights[height][0]);
        wb_motor_set_position(arm[2].motor, heights[height][1]);
        wb_motor_set_position(arm[3].motor, heights[height][2]);

        return RUNNING;
    }

    bool isGripRotation(double angle)
    {
        return areSame(wb_position_sensor_get_value(arm[4].sensor), angle);
    }

    Status setGripRotation(double angle)
    {
        if (isGripRotation(angle))
            return SUCCESS;

        wb_motor_set_position(arm[4].motor, angle);
        return RUNNING;
    }

// gripper

    bool isGripped()
    {
        return (wb_position_sensor_get_value(gripper[0].sensor) < 0.0001) && (wb_position_sensor_get_value(gripper[1].sensor) < 0.0001);
    }

    bool isUnGripped()
    {
        return (wb_position_sensor_get_value(gripper[0].sensor) >= 0.0249) && (wb_position_sensor_get_value(gripper[1].sensor) >= 0.0249);
    }

    Status grip()
    {
        if (isGripped())
            return SUCCESS;

        wb_motor_set_position(gripper[0].motor, 0);
        wb_motor_set_position(gripper[1].motor, 0);

        return RUNNING;
    }

    Status unGrip()
    {
        if (isUnGripped())
            return SUCCESS;

        wb_motor_set_position(gripper[0].motor, 0.025);
        wb_motor_set_position(gripper[1].motor, 0.025);

        return RUNNING;
    }

// supervisor

    std::vector<double> getPos(Object obj)
    {
        const double* pos = wb_supervisor_field_get_sf_vec3f(obj.trans);
        return {pos[0], pos[1], pos[2]};
    }

    double getRot(Object obj)
    {
        const double* rot = wb_supervisor_field_get_sf_rotation(obj.rot);
        return rot[3] * ((rot[2] > 0) - (rot[2] < 0)); // correct axis
    }

    double getRelAng(Object obj)
    {
        std::vector<double> pos = getPos(robot), otherPos = getPos(obj);
        double facing = getRot(robot), toOther = atan2(otherPos[1] - pos[1], otherPos[0] - pos[0]);;
        return atan2(sin(toOther - facing), cos(toOther - facing));
    }

    double getDist(Object a, Object b)
    {
        std::vector<double> aPos = getPos(a), bPos = getPos(b);
        return std::sqrt(pow(bPos[1] - aPos[1], 2) + pow(bPos[0] - aPos[0], 2));
    }

// nodes

    bool isFacing(Object obj)
    {
        return std::abs(getRelAng(obj)) <= 0.01;
    }

    Status face(Object obj)
    {
        if (isFacing(obj))
            return SUCCESS;

        turnRight(getRelAng(obj) * -4);
        return RUNNING;
    }

    bool isAt(Object obj)
    {
        return areSame(getDist(robot, obj), 0.395 + 0.055 * (obj == goal), 0.01);
    }

    Status approach(Object obj)
    {
        if (isAt(obj))
            return SUCCESS;

        moveForward((getDist(robot, obj) - (0.395 + 0.055 * (obj == goal))) * 4);
        return RUNNING;
    }

    bool isGrippedAround(Object obj)
    {
        if (obj == ball)
            return isGripRotation(0);

        double robAng = getRot(robot), otherAng = getRot(obj);
        double ang = atan2(sin(otherAng + robAng), cos(otherAng + robAng));

        for (int i = 0; i < 4; i++)
        {   
            if (isGripRotation(ang))
                return true;

            ang = atan2(sin(ang + M_PI_2), cos((ang + M_PI_2)));
        }

        return false;
    }

    Status gripAround(Object obj)
    {
        if (isGrippedAround(obj))
            return SUCCESS;

        if (obj == ball)
        {
            wb_motor_set_position(arm[4].motor, 0);
            return RUNNING;
        }

        double robAng = getRot(robot), otherAng = getRot(obj);
        double ang = atan2(sin(otherAng + robAng), cos(otherAng + robAng));
        double minAng = ang, minAbs = std::abs(ang);
        
        for (int i = 1; i < 4; i++)
        {
            ang = atan2(sin(ang + M_PI_2), cos((ang + M_PI_2)));

            if (abs(ang) < minAbs)
            {
                minAng = ang;
                minAbs = abs(ang);
            }
        }

        setGripRotation(minAng);
        return RUNNING;
    }

    bool isGrabbed(Object obj)
    {
        if (obj == objects[0])
            return wb_position_sensor_get_value(gripper[0].sensor) + wb_position_sensor_get_value(gripper[1].sensor) < 0.0175;
        else if (obj == objects[1])
            return wb_position_sensor_get_value(gripper[0].sensor) + wb_position_sensor_get_value(gripper[1].sensor) < 0.01;

        return false;
    }

    Status grab(Object obj)
    {
        if (isGrabbed(obj))
            return SUCCESS;

        grip();
        return RUNNING;
    }

    bool isHeld(Object obj)
    {
        return getPos(obj)[2] > 0.021 && std::abs(getRelAng(obj)) < M_PI_2;
    }

    bool isOnGoal(Object obj)
    {
        return getPos(obj)[2] < 0.021 && getDist(obj, goal) < 0.25;
    }

    bool isFacingAway(Object obj)
    {
        return abs(getRelAng(obj)) > M_PI_4;
    }

    Status faceAway(Object obj)
    {
        if (isFacingAway(obj))
                return SUCCESS;

        turnRight(1.0 / getRelAng(obj));
        return RUNNING;
    }

    bool isntBlocked(Object obj)
    {
        for (Object obs : objects)
        {
            if (obs == obj)
                continue;

            if (getDist(robot, obs) < 0.5 && std::abs(getRelAng(obs)) < M_PI_2)
                return false;
        }

        return true;
    }
};