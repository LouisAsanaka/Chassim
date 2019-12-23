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

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            ui.handleEvent(event);
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

        ui.update();
        ui.draw();
        window.display();
    }

    return 0;
}
