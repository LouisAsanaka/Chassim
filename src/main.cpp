#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <iostream>
#include "constants.hpp"
#include "pathgen.hpp"
#include "environment.hpp"
#include "robot.hpp"
#include "utils.hpp"

static const float MAX_SPEED = 1.6f; // in m/s

int main() {
    // Prepare the window
    const int WIDTH = 1800;
    const int HEIGHT = 730;
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT, 32), "Chassim", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::View view;
    view.setCenter(sf::Vector2f{WIDTH / 2, HEIGHT / 2});
    view.setSize(sf::Vector2f{WIDTH, HEIGHT});
    window.setView(view);

    const int CENTER_X = sf::VideoMode::getDesktopMode().width / 2 - WIDTH / 2;
    const int CENTER_Y = sf::VideoMode::getDesktopMode().height / 2 - HEIGHT / 2;
    window.setPosition(sf::Vector2i(CENTER_X, CENTER_Y));

    // Create components
    Environment env;
    //Robot robot{env, 306, 550};
    Robot robot{env, 386, 364};
    //Robot robot{env, 1420, 364};
    b2Body* robotBody{robot.getBody()};

    PathGenerator pathGen{ROBOT_PHYSICAL_SIZE, 3.0, 4.0, 20.0};
    // +x forward, +y left
    TrajectoryPair* traj = pathGen.generatePath({
        Point{0, 0, 0},
        Point{1, 1, b2_pi / 2},
        Point{2, 2, 0},
        Point{9, 2, 0},
        Point{10, 1, b2_pi / 2},
        Point{11, 0, 0},
        });
    int length = traj->length;
    bool isRunning = false;

    int i = 0;
    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window: exit
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }


        /*if (!isRunning && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            isRunning = true;
            i = 0;
            robot.setPosition(386, 364);
        }*/

        if (isRunning) {
            if (i < length) {
                robot.setWheelSpeeds(traj->left[i].velocity, traj->right[i].velocity);
                ++i;
            } else {
                isRunning = false;
            }
        }

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

        /*auto mousePos = sf::Mouse::getPosition(window);
        b2Vec2 vec = b2Vec2{P2M((float) mousePos.x), P2M((float) mousePos.y)} -
            robotBody->GetPosition();
        vec.Normalize();
        vec *= MAX_SPEED;
        robotBody->ApplyForceToCenter(vec, true);*/

        //std::cout << mousePos.x << ", " << mousePos.y << std::endl;

        env.update();
        robot.update();

        window.clear(sf::Color::White);

        env.render(window);
        robot.render(window);

        window.display();
    }

    delete traj->left;
    delete traj->right;
    delete traj;

    return 0;
}
