#include "pointsList.hpp"

#include <TGUI/TGUI.hpp>
#include <vector>
#include <numeric>
#include <sstream>

#include "constants.hpp"
#include "structs.hpp"
#include "utils.hpp"

PointsList::PointsList() {

}

int PointsList::addPoint(double x, double y, double theta) {
    points.emplace_back(x, y, theta);
    return points.size() - 1;
}

int PointsList::addPoint(sf::String str) {
    auto data = parsePointStr(str, ',');
    if (data.size() == 2) {
        points.emplace_back(ROUND(data[0]), ROUND(data[1]));
    } else if (data.size() == 3) {
        points.emplace_back(ROUND(data[0]), ROUND(data[1]), ROUND(data[2]));
    } else {
        return -1;
    }
    return points.size() - 1;
}

void PointsList::setPoint(int index, double x, double y, double theta) {
    Point& point = points.at(index);
    point.x = ROUND(x);
    point.y = ROUND(y);
    point.theta = ROUND(theta);
}

bool PointsList::setPoint(int index, sf::String str) {
    auto data = parsePointStr(str, ',');
    if (data.size() == 2) {
        setPoint(index, data[0] * INCHES2METERS, data[1] * INCHES2METERS);
    } else if (data.size() == 3) {
        setPoint(index, data[0] * INCHES2METERS, data[1] * INCHES2METERS, data[2]);
    } else {
        return false;
    }    
    return true;
}

void PointsList::removePoint(int index) {
    points.erase(points.begin() + index);
}

void PointsList::swap(int index1, int index2) {
    std::iter_swap(points.begin() + index1, points.begin() + index2);
}

const Point& PointsList::getPoint(int index) {
    return points.at(index);
}

const std::vector<Point>& PointsList::getPoints() {
    return points;
}

int PointsList::size() {
    return points.size();
}

std::vector<double> PointsList::parsePointStr(sf::String str, char delim) {
    str.replace(" ", "");

    std::vector<double> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delim)) {
        try {
            tokens.push_back(std::stod(token));
        } catch (std::invalid_argument) {
            tokens.clear();
            return tokens;
        }
    }
    return tokens;
}

std::vector<sf::String> PointsList::toStrVector(const Point& point) {
    std::vector<sf::String> v;
    v.emplace_back(ROUND2STR(point.x * METERS2INCHES));
    v.emplace_back(ROUND2STR(point.y * METERS2INCHES));
    if (std::isnan(point.theta)) {
        v.emplace_back();
    } else {
        v.emplace_back(ROUND2STR(point.theta));
    }
    return v;
}