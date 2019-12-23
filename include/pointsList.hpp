#pragma once

#include <TGUI/TGUI.hpp>
#include "structs.hpp"
#include <vector>

class PointsList {
public:
    PointsList();

    int addPoint(double x, double y, double theta = 0.0);
    int addPoint(sf::String str);
    void setPoint(int index, double x, double y, double theta = 0.0);
    bool setPoint(int index, sf::String str);
    void removePoint(int index);
    void swap(int index1, int index2);
    const Point& getPoint(int index);
    const std::vector<Point>& getPoints();
    int size();

    static std::vector<double> parsePointStr(sf::String str, char delim);
    static std::vector<sf::String> toStrVector(const Point& point);
private:
    std::vector<Point> points;
};