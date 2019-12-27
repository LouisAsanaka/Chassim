#pragma once

#include <TGUI/TGUI.hpp>
#include <vector>

#include "structs.hpp"
#include "pointsList.hpp"
#include "field.hpp"
#include "environment.hpp"
#include "sfLine.hpp"
#include "pathgen.hpp"
#include "robot.hpp"

extern sf::Cursor defaultCursor;
extern sf::Cursor grabCursor;

#define b2_twopi 6.28318530718f

class SimController {
public:
    SimController(sf::RenderWindow& window, Field& field);

    void handleEvent(sf::Event event);
    void update();
    void draw();

    void itemSelected(int index);
    void editRow(int index);
    void commitChange(const sf::String& str);
    void unfocused();

    void clearPoints();
    void resetRobot();
    void generateProfile();
    void executeProfile();

    void addPoint(float meterX, float meterY, int pixelX, int pixelY);
    void addPoint(float meterX, float meterY);
    void setPoint(int index, int pixelX, int pixelY, bool draw = false);
    void setPoint(int index, const sf::String& str);
    void swapPoints(int index1, int index2);
    void removePoint(int index);
    sf::Vector2f pixelsRelativeToOrigin(float meterX, float meterY);
    sf::Vector2f metersRelativeToOrigin(int pixelX, int pixelY);
    std::vector<Point> getPoints();

    static inline float normalizeAngle(float angle) {
        float newAngle = angle;
        while (newAngle <= -b2_pi) newAngle += b2_twopi;
        while (newAngle > b2_pi) newAngle -= b2_twopi;
        return newAngle;
    }
private:
    void createComponents();

    sf::RenderWindow& window;
    tgui::Gui gui;

    Field& field;
    Environment env;
    Robot robot;

    PathGenerator pathGen;
    TrajectoryPair* traj = nullptr;
    std::vector<sfLine> splineLines;
    int trajIndex = 0;
    bool isPathing = false;

    PointsList points;
    std::vector<sf::CircleShape> pointSprites;

    int prevSelectedIndex = -1;

    int pointDraggingIndex = -1;
    bool isDraggingPoint = false;
};