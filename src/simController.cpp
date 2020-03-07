#include "simController.hpp"

#include <TGUI/TGUI.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <json.hpp>

#include "constants.hpp"
#include "field.hpp"
#include "structs.hpp"
#include "pointsList.hpp"
#include "sfLine.hpp"
#include "utils.hpp"

sf::Cursor defaultCursor;
sf::Cursor grabCursor;

SimController::SimController(sf::RenderWindow& window, Field& field) :
    window{window},
    gui{window},
    field{field},
    env{field},
    robot{env, field.getSpawnPoint().x, field.getSpawnPoint().y},
    pathGen{[this](const char* bytes, size_t n) { return this->fillPath(bytes, n); }},
    task{&SimController::executeProfileThread, this} {
    defaultCursor.loadFromSystem(sf::Cursor::Type::Arrow);
    grabCursor.loadFromSystem(sf::Cursor::Type::Hand);

    splineLines.reserve(1000);
    trajectory.reserve(1000);
    createComponents();

    addPoint(0, 0, field.getOrigin().x, field.getOrigin().y + MENU_BAR_HEIGHT);
}

SimController::~SimController() {
    destructed.store(true, std::memory_order_release);
    task.join();
}

void SimController::handleEvent(sf::Event event) {
    if (isPathing.load(std::memory_order_acquire)) {
        gui.handleEvent(event);
        return;
    }
    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    int index = pointsList->getSelectedItemIndex();
    switch (event.type) {
    case sf::Event::EventType::KeyReleased:
        switch (event.key.code) {
        case sf::Keyboard::Delete:
            if (index != -1 && index != 0 && gui.get<tgui::EditBox>("rowEditBox") == nullptr) {
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
        case sf::Keyboard::C:
            if (event.key.control && event.key.shift) {
                copyPoints();
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
            } else if (gui.get<tgui::EditBox>("rowEditBox") == nullptr) {
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
            }
        }
    }
}

void SimController::update(float dt) {
    std::lock_guard<std::mutex> lock{robotMutex};
    if (!isPathing.load(std::memory_order_acquire)) {
        // Manual control
        float left = 0.0f;
        float right = 0.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            left = 3.0f;
            right = 3.0f;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            left = -3.0f;
            right = -3.0f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            left -= 3.0f;
            right += 3.0f;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            left += 3.0f;
            right -= 3.0f;
        }
        robot.setWheelSpeeds(left, right);
    }
    robot.update(dt);
    env.update(dt);
}

void SimController::draw() {
    // Render the sprites
    window.clear(sf::Color::White);

    env.render(window);

    robotMutex.lock();
    robot.render(window);
    robotMutex.unlock();

    for (auto& pointSprite: pointSprites) {
        window.draw(pointSprite);
    }

    if (!splineLines.empty()) {
        for (auto& splineLine : splineLines) {
            splineLine.draw(window, sf::RenderStates::Default);
        }
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
    if (isPathing.load(std::memory_order_acquire)) {
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
    for (int i = points.size() - 1; i >= 0; --i) {
        removePoint(i);
    }
    splineLines.clear();
    trajectory.clear();
    addPoint(0, 0, field.getOrigin().x, field.getOrigin().y + MENU_BAR_HEIGHT);

    while (isBuffering.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    bufferedResponse = "";
}

void SimController::copyPoints() {
    std::ostringstream data;
    data << "{\n";

    auto& points = getPoints();
    int len = points.size();
    int i = 0;
    for (const Point& point : points) {
        if (std::isnan(point.theta)) {
            data << "    new Point(";
            data << ROUND(point.x * METERS2INCHES);
            data << ", ";
            data << ROUND(point.y * METERS2INCHES);
        } else {
            data << "    new Point(";
            data << ROUND(point.x * METERS2INCHES);
            data << ", ";
            data << ROUND(point.y * METERS2INCHES);
            data << ", ";
            data << ROUND(point.theta);
        }
        if (i == len - 1) {
            data << ")\n";
        } else {
            data << "),\n";
        }
    }
    data << "}";
    sf::Clipboard::setString(data.str());
}

void SimController::resetRobot() {
    isPathing.store(false, std::memory_order_release);
    
    std::lock_guard<std::mutex> lock{robotMutex};
    auto& origin = field.getSpawnPoint();
    robot.setPixelPosition(
        origin.x, origin.y
    );
    robot.stop();
}

void SimController::generateProfile() {
    if (!splineLines.empty()) {
        splineLines.clear();
    }
    if (!trajectory.empty()) {
        trajectory.clear();
    }
    if (points.size() < 2) {
        return;
    }
    while (isBuffering.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    pathGen.send(points.getPoints(), 8, 8);
}

void SimController::fillPath(const char* bytes, size_t n) {
    if (bufferedResponse.empty()) {
        isBuffering.store(true, std::memory_order_release);
    }

    // Comes from a buffer, so look for the end of buffer
    auto data = std::string(bytes, n);
    bufferedResponse += data;

    // End of buffer!
    if (data[n - 2] == '\r' && data[n - 1] == '\n') {
        formPath();
        bufferedResponse = "";
        isBuffering.store(false, std::memory_order_release);
    }
}

void SimController::formPath() {
    std::cout << bufferedResponse << std::endl;

    auto fullData = nlohmann::json::parse(bufferedResponse);
    auto& pathPoints = fullData["path"];

    int size = pathPoints.size();
    if (splineLines.size() < size) {
        splineLines.reserve(size);
    }
    if (trajectory.size() < size) {
        trajectory.reserve(size);
    }
    auto& origin = field.getOrigin();
    for (int i = 0; i < size; ++i) {
        auto& point = pathPoints[i];
        trajectory.emplace_back(
            point["velocity"].get<float>() * INPP100MS2MPS,
            point["angle"].get<float>() * b2_pi / 180,
            point["time"].get<float>() * HUNDREDMS2S
        );
        if (i == size - 1) {
            break;
        }
        auto& nextPoint = pathPoints[i + 1];
        splineLines.emplace_back(
            origin + sf::Vector2f{
                field.m2pX(point["x"].get<float>() * INCHES2METERS),
                -field.m2pY(point["y"].get<float>() * INCHES2METERS) + MENU_BAR_HEIGHT
            },
            origin + sf::Vector2f{
                field.m2pX(nextPoint["x"].get<float>() * INCHES2METERS),
                -field.m2pY(nextPoint["y"].get<float>() * INCHES2METERS) + MENU_BAR_HEIGHT
            }
        );
    }
}

void SimController::runProfile() {
    isPathing.store(true, std::memory_order_release);
}

void SimController::executeProfileThread() {
    while (!destructed.load(std::memory_order_acquire)) {
        if (isPathing.load(std::memory_order_acquire)) {
            std::cout << "Running profile" << std::endl;
            executeProfile();
            std::cout << "Finished profile" << std::endl;
            isPathing.store(false, std::memory_order_release);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void SimController::executeProfile() {
    int pathLength = trajectory.size();
    auto& startingPos = points.getPoint(0);
    robotMutex.lock();
    robot.setMeterPosition(
        startingPos.x,
        startingPos.y,
        trajectory[0].angle
    );
    robotMutex.unlock();
    
    for (int i = 0; i < pathLength - 1 && isPathing.load(std::memory_order_acquire); ++i) {
        float dt = trajectory[i + 1].dt;
        float linearSpeed = trajectory[i].velocity;
        float angularSpeed = (trajectory[i + 1].angle - trajectory[i].angle) / dt;

        //std::cout << "Robot Angle: " << robot.getBody()->GetAngle() * 180 / b2_pi << std::endl;
        //std::cout << "Target Angle: " << trajectory[i + 1].angle * 180 / b2_pi << std::endl;
        //std::cout << linearSpeed << " | " << trajectory[i].velocity << std::endl;

        robotMutex.lock();
        robot.setChassisSpeeds(linearSpeed, angularSpeed);
        //robot.update(dt);
        robotMutex.unlock();

        // sleep_until isn't precise enough, so gotta CPU hog cycles now
        // TODO: Find a better way of doing this
        auto start = std::chrono::steady_clock::now();
        while (true) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
            if (elapsed.count() * 1e-6 > dt) {
                break;
            }
            //std::this_thread::sleep_for(std::chrono::duration<float>(dt / 10));
        }
        //std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::duration<float>(dt));
    }
}

void SimController::addPoint(float meterX, float meterY, int pixelX, int pixelY) {
    // Model point
    points.addPoint(meterX, meterY);

    // Table point
    tgui::ListView::Ptr pointsList = gui.get<tgui::ListView>("pointsList");
    pointsList->addItem({
        ROUND2STR(meterX * METERS2INCHES), 
        ROUND2STR(meterY * METERS2INCHES),
        ""
    });

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
    auto meters = metersRelativeToOrigin(pixelX, pixelY);
    points.setPoint(index, meters.x, meters.y, points.getPoint(index).theta);

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
    sf::Vector2f pos{field.m2pX(meterX), -field.m2pY(meterY)};
    pos += field.getOrigin();
    pos.y += MENU_BAR_HEIGHT;
    return pos;
}

sf::Vector2f SimController::metersRelativeToOrigin(int pixelX, int pixelY) {
    auto& origin = field.getOrigin();
    return {field.p2mX(pixelX - origin.x), field.p2mY(origin.y + MENU_BAR_HEIGHT - pixelY)};
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
    menuBar->addMenuItem("Copy Points");
    menuBar->connectMenuItem("Edit", "Copy Points", &SimController::copyPoints, this);
    menuBar->addMenuItem("Reset Robot");
    menuBar->connectMenuItem("Edit", "Reset Robot", &SimController::resetRobot, this);
    menuBar->addMenu("Pathing");
    menuBar->addMenuItem("Generate Profile");
    menuBar->connectMenuItem("Pathing", "Generate Profile", &SimController::generateProfile, this);
    menuBar->addMenuItem("Execute Profile");
    menuBar->connectMenuItem("Pathing", "Execute Profile", &SimController::runProfile, this);
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
    pointsList->addColumn("X (in)", POINTS_LIST_COLUMN_WIDTH);
    pointsList->addColumn("Y (in)", POINTS_LIST_COLUMN_WIDTH);
    // https://www.sfml-dev.org/tutorials/2.5/graphics-text.php
    pointsList->addColumn(L"\u03B8 (\u00B0)", POINTS_LIST_COLUMN_WIDTH);

    gui.add(pointsList, "pointsList");
}