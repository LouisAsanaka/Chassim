#include "pointsList.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

PointsList::PointsList() {

}

int PointsList::addPoint(double x, double y, double theta) {
    points.emplace_back(x, y, theta);
    return points.size() - 1;
}

int PointsList::addPoint(sf::String str) {
    auto data = parsePointStr(str, ',');
    if (data.size() != 3) {
        return -1;
    }
    points.emplace_back(data[0], data[1], data[2]);
    return points.size() - 1;
}

void PointsList::setPoint(int index, double x, double y, double theta) {
    Point& point = points.at(index);
    point.x = x;
    point.y = y;
    point.theta = theta;
}

bool PointsList::setPoint(int index, sf::String str) {
    auto data = parsePointStr(str, ',');
    if (data.size() != 3) {
        return false;
    }
    setPoint(index, data[0], data[1], data[2]);
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
    v.emplace_back(std::to_string(point.x));
    v.emplace_back(std::to_string(point.y));
    v.emplace_back(std::to_string(point.theta));
    return v;
}