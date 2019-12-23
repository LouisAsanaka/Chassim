#include "main.hpp"
#include <TGUI/TGUI.hpp>
#include <Box2D/Box2D.h>
#include <iostream>
#include <vector>
#include "constants.hpp"
#include "pathgen.hpp"
#include "environment.hpp"
#include "robot.hpp"
#include "utils.hpp"
#include "uiController.hpp"

sf::Texture field;

void initTextures() {
    field.loadFromFile("asset/field.png");
}

int main() {
    initTextures();

    // Initialize the window
    sf::Vector2u windowSize{field.getSize()};
    windowSize += sf::Vector2u{POINTS_LIST_WIDTH, MENU_BAR_HEIGHT};

    sf::RenderWindow window{
        sf::VideoMode(windowSize.x, windowSize.y, 32),
        "Chassim", sf::Style::Titlebar | sf::Style::Close
    };
    window.setFramerateLimit(60);

    const int CENTER_X = sf::VideoMode::getDesktopMode().width / 2 - windowSize.x / 2;
    const int CENTER_Y = sf::VideoMode::getDesktopMode().height / 2 - windowSize.y / 2;
    window.setPosition(sf::Vector2i(CENTER_X, CENTER_Y));

    // Create the GUI
    UIController ui{window};

    // Create simulator components
    Environment env{field};
    Robot robot{env, 196, 364};
    b2Body* robotBody{robot.getBody()};

    PathGenerator pathGen{ROBOT_PHYSICAL_SIZE, 3.0, 4.0, 20.0};

    std::vector<Point> waypoints = {
        Point{0, 0, 0},
        Point{1, 1, b2_pi / 2},
        Point{2, 2, 0},
        Point{9, 2, 0},
        Point{10, 1, b2_pi / 2},
        Point{11, 0, 0}
    };
    
    // +x forward, +y left
    TrajectoryPair* traj = pathGen.generatePath(waypoints);
    int length = traj->length;
    bool isRunning = false;
    int i = 0;

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            ui.handleEvent(event);
        }

        if (!isRunning && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            isRunning = true;
            i = 0;
            robot.setPosition(196, 364);
        }

        if (isRunning) {
            if (i < length) {
                robot.setWheelSpeeds(traj->left[i].velocity, traj->right[i].velocity);
                ++i;
            } else {
                isRunning = false;
            }
        }

        /*float left = 0.0f;
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
        robot.setWheelSpeeds(left, right);*/

        // Do physics updates
        env.update();
        robot.update();

        // Render the sprites
        window.clear(sf::Color::White);

        env.render(window);
        robot.render(window);

        ui.draw();
        window.display();
    }

    delete traj->left;
    delete traj->right;
    delete traj;

    return 0;
}
