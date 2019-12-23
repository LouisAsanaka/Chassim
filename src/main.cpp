#include "main.hpp"

#include <SFML/Graphics.hpp>

#include "simController.hpp"

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

    SimController controller{window};

    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            controller.handleEvent(event);
        }

        controller.update();
        controller.draw();
        window.display();
    }
}
