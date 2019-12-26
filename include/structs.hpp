#pragma once

#include <numeric>

#define NO_ANGLE std::numeric_limits<double>::quiet_NaN()

struct Point {
    double x;
    double y;
    double theta;

    Point(double x, double y, double theta = NO_ANGLE) :
        x{x}, y{y}, theta{theta} {

    }
};