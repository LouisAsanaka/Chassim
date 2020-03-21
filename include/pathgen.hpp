#pragma once

#include <vector>

#include "structs.hpp"
#include "pathfinder/pathfinder.h"

struct TrajectoryPair {
    Segment* left;
    Segment* right;
    Segment* original;
    int length;
    double totalTime;
    double pathLength;
};

class PathGenerator {
public:
    PathGenerator(float trackwidth);

    TrajectoryPair* generatePath(std::vector<Point> waypoints, 
        double maxVel, double maxAccel, double maxJerk);
private:
    float trackwidth;
};