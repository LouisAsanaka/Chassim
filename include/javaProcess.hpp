#pragma once

#include <TinyProcessLib/process.hpp>
#include <vector>

#include "structs.hpp"

#define PROCESS_PATH "java\\build\\install\\chassim\\bin\\chassim.bat"

class JavaProcess {
public:
    JavaProcess(std::function<void(const char* bytes, size_t n)> callback);
    ~JavaProcess();

    void send(const std::vector<Point>& points, float cruiseVelocity, float targetAcceleration);
private:
    TinyProcessLib::Process process;
};