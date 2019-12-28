#include "main.hpp"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>

#include "simController.hpp"
#include "constants.hpp"
#include "field.hpp"

int main() {
    Field field;

    // Initialize the window
    sf::Vector2u windowSize{field.getTexture().getSize()};
    windowSize += sf::Vector2u{POINTS_LIST_WIDTH, MENU_BAR_HEIGHT};

    sf::RenderWindow window{
        sf::VideoMode(windowSize.x, windowSize.y, 32),
        "Chassim", sf::Style::Titlebar | sf::Style::Close
    };
    window.setFramerateLimit(FRAMERATE);

    const int CENTER_X = sf::VideoMode::getDesktopMode().width / 2 - windowSize.x / 2;
    const int CENTER_Y = sf::VideoMode::getDesktopMode().height / 2 - windowSize.y / 2;
    window.setPosition(sf::Vector2i(CENTER_X, CENTER_Y));

    SimController controller{window, field};

    sf::Clock clock;
    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            controller.handleEvent(event);
        }
        // Ref: https://gafferongames.com/post/fix_your_timestep/
        float frameTime = clock.restart().asSeconds();
        int i = 0;
        while (frameTime > 0.0f) {
            float deltaTime = std::min(frameTime, IDEAL_DT);
            controller.update(deltaTime);
            frameTime -= deltaTime;
        }
        controller.draw();
        window.display();
    }
}
