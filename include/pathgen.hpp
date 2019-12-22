#pragma once
#include <initializer_list>
#include <string>
#include "pathfinder/pathfinder.h"

struct Point {
    float x;
    float y;
    float theta;
};

struct TrajectoryPair {
    Segment* left;
    Segment* right;
    int length;
};

class PathGenerator {
public:
    PathGenerator(float trackwidth, double maxVel, double maxAccel, double maxJerk);

    TrajectoryPair* generatePath(std::initializer_list<Point> waypoints);
private:
    float trackwidth;
    double maxVel;
    double maxAccel;
    double maxJerk;
};