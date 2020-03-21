#include "pathgen.hpp"

#include <iostream>
#include <numeric>
#include <vector>
#include <string>

#include "structs.hpp"
#include "pathfinder/pathfinder.h"

PathGenerator::PathGenerator(float trackwidth) :
    trackwidth{trackwidth} {

}

/**
 * Original author: Ryan Benasutti, WPI
 * Modified from OkapiLib
 */
TrajectoryPair* PathGenerator::generatePath(std::vector<Point> waypoints,
    double maxVel, double maxAccel, double maxJerk 
) {
    if (waypoints.size() == 0) {
        // No point in generating a path
        std::cout <<
            "PathGenerator: Not generating a path because no waypoints were given." << std::endl;
        return nullptr;
    }

    std::vector<Waypoint> points;
    points.reserve(waypoints.size());
    for (auto& point : waypoints) {
        points.push_back(
            Waypoint{point.x, point.y, point.theta});
    }

    TrajectoryCandidate candidate;
    std::cout << "PathGenerator: Preparing trajectory" << std::endl;
    pathfinder_prepare(points.data(),
        static_cast<int>(points.size()),
        FIT_HERMITE_CUBIC,
        PATHFINDER_SAMPLES_FAST,
        1.0 / 60,
        maxVel,
        maxAccel,
        maxJerk,
        &candidate);
    const int length = candidate.length;
    const double totalTime = candidate.length * candidate.info.dt;
    const double pathLength = candidate.totalLength;

    if (length < 0) {
        auto pointToString = [](Waypoint point) {
            return "Point{x = " + std::to_string(point.x) + ", y = " + std::to_string(point.y) +
                ", theta = " + std::to_string(point.angle) + "}";
        };

        std::string message =
            "PathGenerator: Path is impossible with waypoints: " +
            std::accumulate(std::next(points.begin()),
                points.end(),
                pointToString(points.at(0)),
                [&](std::string a, Waypoint b) { return a + ", " + pointToString(b); });

        std::cout << message << std::endl;

        if (candidate.laptr) {
            free(candidate.laptr);
        }

        if (candidate.saptr) {
            free(candidate.saptr);
        }

        throw std::runtime_error(message);
    }

    auto* trajectory = static_cast<Segment*>(malloc(length * sizeof(Segment)));

    if (trajectory == nullptr) {
        std::string message = "PathGenerator: Could not allocate trajectory. The path "
            "is probably impossible.";
        std::cout << message << std::endl;

        if (candidate.laptr) {
            free(candidate.laptr);
        }

        if (candidate.saptr) {
            free(candidate.saptr);
        }

        throw std::runtime_error(message);
    }

    std::cout << "PathGenerator: Generating path" << std::endl;
    pathfinder_generate(&candidate, trajectory);

    auto* leftTrajectory = (Segment*)malloc(sizeof(Segment) * length);
    auto* rightTrajectory = (Segment*)malloc(sizeof(Segment) * length);

    if (leftTrajectory == nullptr || rightTrajectory == nullptr) {
        std::string message = "PathGenerator: Could not allocate left and/or right "
            "trajectories. The path is probably impossible.";
        std::cout << message << std::endl;

        if (leftTrajectory) {
            free(leftTrajectory);
        }

        if (rightTrajectory) {
            free(rightTrajectory);
        }

        if (trajectory) {
            free(trajectory);
        }

        throw std::runtime_error(message);
    }

    std::cout << "PathGenerator: Modifying for tank drive" << std::endl;
    pathfinder_modify_tank(
        trajectory, length, leftTrajectory, rightTrajectory, trackwidth);

    std::cout << "PathGenerator: Completely done generating path" << std::endl;
    std::cout << "PathGenerator: " + std::to_string(length) << std::endl;

    return new TrajectoryPair{leftTrajectory, rightTrajectory, trajectory, length,
        totalTime, pathLength};
}