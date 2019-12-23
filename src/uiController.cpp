#include "uiController.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include "constants.hpp"
#include "utils.hpp"

UIController::UIController(sf::RenderWindow& window) :
    window{window},
    gui{window},
    draggingIndex{NO_DRAGGING_INDEX},
    isDragging{false} {
    createComponents();

    addPoint(0, 0, 196, 364 + MENU_BAR_HEIGHT);
    addPoint(1, 1);
    addPoint(2, 2);
    addPoint(9, 2);
    addPoint(10, 1);
    addPoint(11, 0);
}

void UIController::handleEvent(sf::Event event) {
    // TODO: Remove this hack
    if (event.type == sf::Event::EventType::MouseButtonReleased) {
        if (isDragging) {
            tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
            pointsList->deselectItem();
        }
        draggingIndex = NO_DRAGGING_INDEX;
        isDragging = false;
    } else if (event.type == sf::Event::EventType::KeyReleased && 
        !isDragging &&
        event.key.code == sf::Keyboard::Delete
    ) {
        tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
        int index = pointsList->getSelectedItemIndex();
        if (index != -1 && index != 0) {
            removePoint(index);
        }
    }

    if (!gui.handleEvent(event)) {
        // Event was not intercepted, which means the field was the target
        if (event.type == sf::Event::MouseButtonReleased) {
            int pixelX = event.mouseButton.x;
            int pixelY = event.mouseButton.y;

            auto meters = metersRelativeToOrigin(pixelX, pixelY);
            addPoint(meters.x, meters.y, pixelX, pixelY);
        }
    }
}

void UIController::draw() {
    for (auto& pointSprite: pointSprites) {
        window.draw(pointSprite);
    }
    gui.draw();
}

void UIController::itemSelected(int index) {
    if (index == -1 || index == 0) {
        return;
    }
    if (draggingIndex != NO_DRAGGING_INDEX && 
        draggingIndex != index) {
        isDragging = true;

        points.swap(draggingIndex, index);

        tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
        auto& temp = pointsList->getItemRow(draggingIndex);
        pointsList->changeItem(draggingIndex, pointsList->getItemRow(index));
        pointsList->changeItem(index, temp);

        std::iter_swap(pointSprites.begin() + draggingIndex, pointSprites.begin() + index);
    }
    draggingIndex = index;
}

void UIController::editRow(int index) {
    // Can't drag while editing!
    isDragging = false;
    draggingIndex = NO_DRAGGING_INDEX;

    if (index == 0) {
        return;
    }

    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");

    tgui::EditBox::Ptr editBox = tgui::EditBox::create();
    editBox->setSize({POINTS_LIST_WIDTH, 30});
    editBox->setPosition({
        pointsList->getPosition().x, 
        MENU_BAR_HEIGHT + index * pointsList->getItemHeight() + 
            pointsList->getHeaderHeight() + pointsList->getHeaderSeparatorHeight() * 2});
    editBox->setTextSize(24);
    editBox->limitTextWidth();
    editBox->setText(
        pointsList->getItemCell(index, 0) + ", " +
        pointsList->getItemCell(index, 1) + ", " +
        pointsList->getItemCell(index, 2)
    );
    editBox->setUserData(index);
    editBox->connect("Unfocused", &UIController::unfocused, this);
    editBox->connect("ReturnKeyPressed", &UIController::commitChange, this);
    gui.add(editBox, "rowEditBox");
    editBox->setFocused(true);
}

void UIController::commitChange(const sf::String& str) {
    tgui::EditBox::Ptr editBox = gui.get<tgui::EditBox>("rowEditBox");
    int index = editBox->getUserData<int>();
    if (str.isEmpty()) {
        removePoint(index);
    } else {
        setPoint(index, str);
    }
    editBox->setFocused(false); // Unfocusing the box deletes it
}

void UIController::unfocused() {
    isDragging = false;
    draggingIndex = NO_DRAGGING_INDEX;

    tgui::EditBox::Ptr editBox = gui.get<tgui::EditBox>("rowEditBox");
    gui.remove(editBox);
}

void UIController::addPoint(float meterX, float meterY, int pixelX, int pixelY) {
    // Model point
    points.addPoint(meterX, meterY);

    // Table point
    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    pointsList->addItem({std::to_string(meterX), std::to_string(meterY), "0"});

    // Sprite point
    pointSprites.emplace_back();
    sf::CircleShape& shape = pointSprites.back();
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(POINT_CIRCLE_RADIUS, POINT_CIRCLE_RADIUS);
    shape.setRadius(POINT_CIRCLE_RADIUS);
    shape.setPointCount(60);
    shape.setPosition(pixelX, pixelY);
}

void UIController::addPoint(float meterX, float meterY) {
    auto pixels = pixelsRelativeToOrigin(meterX, meterY);
    addPoint(meterX, meterY, pixels.x, pixels.y);
}

void UIController::setPoint(int index, const sf::String& str) {
    if (points.setPoint(index, str)) {
        tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");

        const Point& point = points.getPoint(index);
        pointsList->changeItem(index, PointsList::toStrVector(point));

        pointSprites.at(index).setPosition(
            pixelsRelativeToOrigin(point.x, point.y)
        );
    }
}

void UIController::removePoint(int index) {
    points.removePoint(index);

    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    pointsList->removeItem(index);

    pointSprites.erase(pointSprites.begin() + index);
}

sf::Vector2f UIController::pixelsRelativeToOrigin(float meterX, float meterY) {
    auto& origin = pointSprites.at(0);
    sf::Vector2f pos{M2P(meterX), -M2P(meterY)};
    std::cout << pos.x << std::endl;
    std::cout << pos.y << std::endl;
    pos += origin.getPosition();
    return pos;
}

sf::Vector2f UIController::metersRelativeToOrigin(int pixelX, int pixelY) {
    auto origin = pointSprites.at(0).getPosition();
    return {P2M(pixelX - origin.x), P2M(origin.y - pixelY)};
}

std::vector<Point> UIController::getPoints() {
    return points.getPoints();
}

void UIController::createComponents() {
    // Make the top menu bar
    tgui::MenuBar::Ptr menuBar = tgui::MenuBar::create();
    menuBar->setTextSize(16);
    menuBar->setSize({"parent.width", MENU_BAR_HEIGHT});
    menuBar->addMenu("File");
    menuBar->addMenuItem("File", "Save As");
    gui.add(menuBar, "menuBar");

    // Make the table of points
    tgui::ListView::Ptr pointsList = tgui::ListView::create();
    pointsList->setTextSize(24);
    pointsList->setSize({POINTS_LIST_WIDTH, "100%"});
    pointsList->setPosition({"parent.width - width", MENU_BAR_HEIGHT});
    pointsList->setHeaderHeight(35);
    pointsList->setHeaderSeparatorHeight(1);
    pointsList->setItemHeight(30);
    pointsList->connect("ItemSelected", &UIController::itemSelected, this);
    pointsList->connect("DoubleClicked", &UIController::editRow, this);

    // Create the columns
    pointsList->addColumn("X (m)", POINTS_LIST_COLUMN_WIDTH);
    pointsList->addColumn("Y (m)", POINTS_LIST_COLUMN_WIDTH);
    // https://www.sfml-dev.org/tutorials/2.5/graphics-text.php
    pointsList->addColumn(L"\u03B8 (\u00B0)", POINTS_LIST_COLUMN_WIDTH);

    gui.add(pointsList, "pointsList");
}