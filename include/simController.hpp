#pragma once

#include <TGUI/TGUI.hpp>
#include <vector>
#include <atomic>
#include <thread>

#include "structs.hpp"
#include "pointsList.hpp"
#include "field.hpp"
#include "environment.hpp"
#include "javaProcess.hpp"
#include "sfLine.hpp"
#include "robot.hpp"

extern sf::Cursor defaultCursor;
extern sf::Cursor grabCursor;

class SimController {
public:
    SimController(sf::RenderWindow& window, Field& field);
    ~SimController();

    void handleEvent(sf::Event event);
    void update(float dt);
    void draw();

    void itemSelected(int index);
    void editRow(int index);
    void commitChange(const sf::String& str);
    void unfocused();

    void clearPoints();
    void copyPoints();
    void resetRobot();
    void generateProfile();
    void fillPath(const char* bytes, size_t n);
    void formPath();
    void runProfile();
    
    void executeProfileThread();
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
private:
    void createComponents();

    sf::RenderWindow& window;
    tgui::Gui gui;

    Field& field;
    Environment env;
    Robot robot;

    JavaProcess pathGen;
    std::string bufferedResponse = "";
    std::atomic_bool isBuffering{false};
    std::vector<sfLine> splineLines{};
    std::vector<TrajectoryPoint> trajectory{};
    
    std::thread task;
    std::atomic_bool isPathing{false};
    std::mutex robotMutex;

    std::atomic_bool destructed{false};

    PointsList points;
    std::vector<sf::CircleShape> pointSprites;

    int prevSelectedIndex = -1;

    int pointDraggingIndex = -1;
    bool isDraggingPoint = false;
};