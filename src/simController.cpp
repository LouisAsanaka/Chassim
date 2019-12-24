#include "simController.hpp"

#include <TGUI/TGUI.hpp>
#include <vector>

#include "constants.hpp"
#include "field.hpp"
#include "structs.hpp"
#include "pointsList.hpp"
#include "pathgen.hpp"

sf::Cursor defaultCursor;
sf::Cursor grabCursor;

SimController::SimController(sf::RenderWindow& window, Field& field) :
    window{window},
    gui{window},
    field{field},
    env{field},
    robot{env, field.getSpawnPoint().x, field.getSpawnPoint().y},
    pathGen{robot.getTrackWidth(), 3.0, 4.0, 20.0} {
    defaultCursor.loadFromSystem(sf::Cursor::Type::Arrow);
    grabCursor.loadFromSystem(sf::Cursor::Type::Hand);

    createComponents();

    addPoint(0, 0, field.getSpawnPoint().x, field.getSpawnPoint().y + MENU_BAR_HEIGHT);
}

void SimController::handleEvent(sf::Event event) {
    if (isPathing) {
        gui.handleEvent(event);
        return;
    }
    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    int index = pointsList->getSelectedItemIndex();
    switch (event.type) {
    case sf::Event::EventType::KeyReleased:
        switch (event.key.code) {
        case sf::Keyboard::Delete:
            if (index != -1 && index != 0) {
                removePoint(index);
                generateProfile();
            }
            break;
        case sf::Keyboard::Up:
            if (index > 1) {
                swapPoints(index, index - 1);
                pointsList->setSelectedItem(index - 1);
                generateProfile();
            }
            break;
        case sf::Keyboard::Down:
            if (index > 0 && index < pointsList->getItemCount() - 1) {
                swapPoints(index, index + 1);
                pointsList->setSelectedItem(index + 1);
                generateProfile();
            }
            break;
        }
        break;
    }

    if (!gui.handleEvent(event)) {
        // Event was not intercepted, which means the field was the target
        switch (event.type) {
        case sf::Event::MouseButtonPressed:
            for (int i = 0; i < pointSprites.size(); ++i) {
                if (pointSprites[i].getGlobalBounds().contains(
                        event.mouseButton.x, event.mouseButton.y)) {
                    isDraggingPoint = true;
                    pointDraggingIndex = i;

                    window.setMouseCursor(grabCursor);
                    break;
                }
            }
            break;
        case sf::Event::MouseButtonReleased:
            if (isDraggingPoint) {
                isDraggingPoint = false;
                pointDraggingIndex = -1;

                window.setMouseCursor(defaultCursor);

                generateProfile();
            } else {
                int pixelX = event.mouseButton.x;
                int pixelY = event.mouseButton.y;
                
                if (window.getViewport(window.getDefaultView()).contains(pixelX, pixelY)) {
                    auto meters = metersRelativeToOrigin(pixelX, pixelY);
                    addPoint(meters.x, meters.y, pixelX, pixelY);
                    generateProfile();
                }
            }
        case sf::Event::MouseMoved:
            if (isDraggingPoint) {
                setPoint(pointDraggingIndex, event.mouseMove.x, event.mouseMove.y, true);
                if (pointDraggingIndex == 0) {
                    for (int i = 1; i < points.size(); ++i) {
                        auto& spritePos = pointSprites.at(i).getPosition();
                        setPoint(i, spritePos.x,
                            spritePos.y, false);
                    }
                }
            }
        }
    }
}

void SimController::update() {
    env.update();

    if (isPathing) {
        if (trajIndex < traj->length) {
            robot.setWheelSpeeds(traj->left[trajIndex].velocity, traj->right[trajIndex].velocity);
            ++trajIndex;
        } else {
            isPathing = false;
        }
    } else {
        // Manual control
        float left = 0.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            left = 2.0f;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            left = -2.0f;
        }
        float right = 0.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            right = 2.0f;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            right = -2.0f;
        }
        robot.setWheelSpeeds(left, right);
    }
    robot.update();
}

void SimController::draw() {
    // Render the sprites
    window.clear(sf::Color::White);

    env.render(window);
    robot.render(window);

    for (auto& pointSprite: pointSprites) {
        window.draw(pointSprite);
    }

    if (splinePoints != nullptr) {
        window.draw(splinePoints, traj->length, sf::Points);
    }
    
    gui.draw();
}

void SimController::itemSelected(int index) {
    if (index == -1) {
        return;
    }
    if (prevSelectedIndex != -1 && prevSelectedIndex < pointSprites.size()) {
        auto& prevSprite = pointSprites.at(prevSelectedIndex);
        prevSprite.setOutlineThickness(0);
    }

    auto& sprite = pointSprites.at(index);
    sprite.setOutlineThickness(4);

    prevSelectedIndex = index;
}

void SimController::editRow(int index) {
    if (isPathing) {
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
    editBox->setText(
        pointsList->getItemCell(index, 0) + ", " +
        pointsList->getItemCell(index, 1) + ", " +
        pointsList->getItemCell(index, 2)
    );
    editBox->setUserData(index);
    editBox->connect("Unfocused", &SimController::unfocused, this);
    editBox->connect("ReturnKeyPressed", &SimController::commitChange, this);
    gui.add(editBox, "rowEditBox");
    editBox->setFocused(true);
}

void SimController::commitChange(const sf::String& str) {
    tgui::EditBox::Ptr editBox = gui.get<tgui::EditBox>("rowEditBox");
    int index = editBox->getUserData<int>();
    if (str.isEmpty()) {
        removePoint(index);
    } else {
        setPoint(index, str);
    }
    generateProfile();
    editBox->setFocused(false); // Unfocusing the box deletes it
}

void SimController::unfocused() {
    tgui::EditBox::Ptr editBox = gui.get<tgui::EditBox>("rowEditBox");
    gui.remove(editBox);
}

void SimController::clearPoints() {
    for (int i = points.size() - 1; i > 0; --i) {
        removePoint(i);
    }
    splinePoints = nullptr;
}

void SimController::resetRobot() {
    isPathing = false;

    auto& origin = pointSprites.at(0).getPosition();
    robot.setPosition(
        origin.x, origin.y - MENU_BAR_HEIGHT, -points.getPoint(0).theta * PI / 180
    );
    robot.stop();
}

void SimController::generateProfile() {
    if (points.size() < 2) {
        return;
    }
    std::vector<Point> waypoints = points.getPoints();
    for (auto& point : waypoints) {
        point.theta = point.theta * PI / 180;
    }
    if (traj != nullptr) {
        delete traj->left;
        delete traj->right;
        delete traj->original;
        delete traj;
    }
    if (splinePoints != nullptr) {
        delete[] splinePoints;
    }
    // TODO: Generate the modified tank trajectory only when executing the profile
    traj = pathGen.generatePath(waypoints);

    splinePoints = new sf::Vertex[traj->length];
    auto origin = pointSprites.at(0).getPosition();
    for (int i = 0; i < traj->length; i++) {
        auto& p = *(traj->original + i);
        splinePoints[i] = sf::Vertex(
            origin + sf::Vector2f{field.m2p(p.x), -field.m2p(p.y)}, sf::Color::Yellow);
    }
}

void SimController::executeProfile() {
    if (!isPathing && traj != nullptr) {
        isPathing = true;
        trajIndex = 0;

        const auto& origin = pointSprites.at(0).getPosition();
        robot.setPosition(
            origin.x, origin.y - MENU_BAR_HEIGHT, -points.getPoint(0).theta * PI / 180
        );
        robot.stop();
    }
}

void SimController::addPoint(float meterX, float meterY, int pixelX, int pixelY) {
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
    shape.setOutlineColor(sf::Color::Red);
}

void SimController::addPoint(float meterX, float meterY) {
    auto pixels = pixelsRelativeToOrigin(meterX, meterY);
    addPoint(meterX, meterY, pixels.x, pixels.y);
}

void SimController::setPoint(int index, int pixelX, int pixelY, bool draw) {
    if (index == 0) {
        points.setPoint(index, 0, 0);
    } else {
        auto meters = metersRelativeToOrigin(pixelX, pixelY);
        points.setPoint(index, meters.x, meters.y);
    }

    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");

    const Point& point = points.getPoint(index);
    pointsList->changeItem(index, PointsList::toStrVector(point));

    if (draw) {
        pointSprites.at(index).setPosition(pixelX, pixelY);
    }
}

void SimController::setPoint(int index, const sf::String& str) {
    if (points.setPoint(index, str)) {
        tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");

        const Point& point = points.getPoint(index);
        pointsList->changeItem(index, PointsList::toStrVector(point));

        pointSprites.at(index).setPosition(
            pixelsRelativeToOrigin(point.x, point.y)
        );
    }
}

void SimController::swapPoints(int index1, int index2) {
    points.swap(index1, index2);

    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    auto& temp = pointsList->getItemRow(index1);
    pointsList->changeItem(index1, pointsList->getItemRow(index2));
    pointsList->changeItem(index2, temp);

    std::iter_swap(pointSprites.begin() + index1, pointSprites.begin() + index2);
}

void SimController::removePoint(int index) {
    points.removePoint(index);

    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    pointsList->removeItem(index);

    pointSprites.erase(pointSprites.begin() + index);
}

sf::Vector2f SimController::pixelsRelativeToOrigin(float meterX, float meterY) {
    auto& origin = pointSprites.at(0);
    sf::Vector2f pos{field.m2p(meterX), -field.m2p(meterY)};
    pos += origin.getPosition();
    return pos;
}

sf::Vector2f SimController::metersRelativeToOrigin(int pixelX, int pixelY) {
    auto& origin = pointSprites.at(0).getPosition();
    return {field.p2m(pixelX - origin.x), field.p2m(origin.y - pixelY)};
}

std::vector<Point> SimController::getPoints() {
    return points.getPoints();
}

void SimController::createComponents() {
    // Make the top menu bar
    tgui::MenuBar::Ptr menuBar = tgui::MenuBar::create();
    menuBar->setTextSize(16);
    menuBar->setSize({"parent.width", MENU_BAR_HEIGHT});
    menuBar->addMenu("Edit");
    menuBar->addMenuItem("Clear Points");
    menuBar->connectMenuItem("Edit", "Clear Points", &SimController::clearPoints, this);
    menuBar->addMenuItem("Reset Robot");
    menuBar->connectMenuItem("Edit", "Reset Robot", &SimController::resetRobot, this);
    menuBar->addMenu("Pathing");
    menuBar->addMenuItem("Generate Profile");
    menuBar->connectMenuItem("Pathing", "Generate Profile", &SimController::generateProfile, this);
    menuBar->addMenuItem("Execute Profile");
    menuBar->connectMenuItem("Pathing", "Execute Profile", &SimController::executeProfile, this);
    gui.add(menuBar, "menuBar");

    // Make the table of points
    tgui::ListView::Ptr pointsList = tgui::ListView::create();
    pointsList->setTextSize(24);
    pointsList->setSize({POINTS_LIST_WIDTH, "100%"});
    pointsList->setPosition({"parent.width - width", MENU_BAR_HEIGHT});
    pointsList->setHeaderHeight(35);
    pointsList->setHeaderSeparatorHeight(1);
    pointsList->setItemHeight(30);
    pointsList->connect("ItemSelected", &SimController::itemSelected, this);
    pointsList->connect("DoubleClicked", &SimController::editRow, this);

    // Create the columns
    pointsList->addColumn("X (m)", POINTS_LIST_COLUMN_WIDTH);
    pointsList->addColumn("Y (m)", POINTS_LIST_COLUMN_WIDTH);
    // https://www.sfml-dev.org/tutorials/2.5/graphics-text.php
    pointsList->addColumn(L"\u03B8 (\u00B0)", POINTS_LIST_COLUMN_WIDTH);

    gui.add(pointsList, "pointsList");
}