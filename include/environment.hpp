#pragma once
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

class Environment {
public:
    Environment();

    b2World& getWorld();
    void update();
    void render(sf::RenderWindow& window);
private:
    void createBoundary(int startX, int startY, int endX, int endY);
    void createRocket(const b2Vec2* points, int length = 6);

    sf::Texture texture;
    sf::Sprite sprite;
    b2World world;
};