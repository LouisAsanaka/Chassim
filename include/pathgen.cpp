#include "pathgen.hpp"
#include <iostream>
#include <vector>
#include <numeric>

PathGenerator::PathGenerator(float trackwidth, double maxVel, double maxAccel, double maxJerk) :
    trackwidth{trackwidth},
    maxVel{maxVel},
    maxAccel{maxAccel},
    maxJerk{maxJerk} {

}

TrajectoryPair* PathGenerator::generatePath(std::initializer_list<Point> waypoints) {
    if (waypoints.size() == 0) {
        // No point in generating a path
        std::cout <<
            "AsyncMotionProfileController: Not generating a path because no waypoints were given." << std::endl;
        return nullptr;
    }

    std::vector<Waypoint> points;
    points.reserve(waypoints.size());
    for (auto& point : waypoints) {
        points.push_back(
            Waypoint{point.x, point.y, point.theta});
    }

    TrajectoryCandidate candidate;
    std::cout << "AsyncMotionProfileController: Preparing trajectory" << std::endl;
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

    if (length < 0) {
        auto pointToString = [](Waypoint point) {
            return "Point{x = " + std::to_string(point.x) + ", y = " + std::to_string(point.y) +
                ", theta = " + std::to_string(point.angle) + "}";
        };

        std::string message =
            "AsyncMotionProfileController: Path is impossible with waypoints: " +
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
        std::string message = "AsyncMotionProfileController: Could not allocate trajectory. The path "
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

    std::cout << "AsyncMotionProfileController: Generating path" << std::endl;
    pathfinder_generate(&candidate, trajectory);

    auto* leftTrajectory = (Segment*)malloc(sizeof(Segment) * length);
    auto* rightTrajectory = (Segment*)malloc(sizeof(Segment) * length);

    if (leftTrajectory == nullptr || rightTrajectory == nullptr) {
        std::string message = "AsyncMotionProfileController: Could not allocate left and/or right "
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

    std::cout << "AsyncMotionProfileController: Modifying for tank drive" << std::endl;
    pathfinder_modify_tank(
        trajectory, length, leftTrajectory, rightTrajectory, trackwidth);

    free(trajectory);

    std::cout << "AsyncMotionProfileController: Completely done generating path" << std::endl;
    std::cout << "AsyncMotionProfileController: " + std::to_string(length) << std::endl;

    return new TrajectoryPair{leftTrajectory, rightTrajectory, length};
}