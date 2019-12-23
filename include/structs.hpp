#pragma once

struct Point {
    double x;
    double y;
    double theta;

    Point(double x, double y, double theta) :
        x{x}, y{y}, theta{theta} {

    }
};