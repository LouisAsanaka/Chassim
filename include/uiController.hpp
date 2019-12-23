#pragma once
#include "structs.hpp"
#include <TGUI/TGUI.hpp>
#include <vector>
#include <string>
#include "pointsList.hpp"

#define NO_DRAGGING_INDEX -2

class UIController {
public:
    UIController(sf::RenderWindow& window);

    void handleEvent(sf::Event event);
    void draw();

    void itemSelected(int index);
    void editRow(int index);
    void commitChange(const sf::String& str);
    void unfocused();

    void addPoint(float meterX, float meterY, int pixelX, int pixelY);
    void addPoint(float meterX, float meterY);
    void setPoint(int index, const sf::String& str);
    void removePoint(int index);
    sf::Vector2f pixelsRelativeToOrigin(float meterX, float meterY);
    sf::Vector2f metersRelativeToOrigin(int pixelX, int pixelY);
    std::vector<Point> getPoints();
private:
    void createComponents();

    sf::RenderWindow& window;
    tgui::Gui gui;

    PointsList points;
    std::vector<sf::CircleShape> pointSprites;

    int draggingIndex;
    bool isDragging;
};