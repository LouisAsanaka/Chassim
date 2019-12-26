#include "javaProcess.hpp"

#include <iostream>
#include <vector>
#include <numeric>
#include <json.hpp>

#include "structs.hpp"
#include "constants.hpp"

JavaProcess::JavaProcess(std::function<void(const char* bytes, size_t n)> callback) :
    process{PROCESS_PATH, ".", callback, nullptr, true} {
}

JavaProcess::~JavaProcess() {
    process.kill();
}

void JavaProcess::send(const std::vector<Point>& points, float cruiseVelocity, float targetAcceleration) {
    nlohmann::json data;
    data["cruiseVelocity"] = cruiseVelocity;
    data["targetAcceleration"] = targetAcceleration;
    data["waypoints"] = nlohmann::json::array();
    for (const Point& point : points) {
        if (std::isnan(point.theta)) {
            data["waypoints"].push_back(
                nlohmann::json::object({
                    {"x", point.x * METERS2INCHES},
                    {"y", point.y * METERS2INCHES}
                })
            );
        } else {
            data["waypoints"].push_back(
                nlohmann::json::object({
                    {"x", point.x * METERS2INCHES},
                    {"y", point.y * METERS2INCHES},
                    {"angle", point.theta}
                })
            );
        }
    }
    process.write(data.dump() + "\n");
}