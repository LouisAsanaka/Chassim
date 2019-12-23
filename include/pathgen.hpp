#pragma once
#include <vector>
#include <string>
#include "structs.hpp"
#include "pathfinder/pathfinder.h"

struct TrajectoryPair {
    Segment* left;
    Segment* right;
    Segment* original;
    int length;
};

class PathGenerator {
public:
    PathGenerator(float trackwidth, double maxVel, double maxAccel, double maxJerk);

    TrajectoryPair* generatePath(std::vector<Point> waypoints);
private:
    float trackwidth;
    double maxVel;
    double maxAccel;
    double maxJerk;
};