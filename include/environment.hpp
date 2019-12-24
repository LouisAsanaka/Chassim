#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "field.hpp"

class Environment {
public:
    Environment(Field& field);

    b2World& getWorld();
    void update();
    void render(sf::RenderWindow& window);
private:
    void createWall(const std::vector<b2Vec2>& vec);
    void createPolygon(const std::vector<b2Vec2>& vec);

    sf::Texture texture;
    sf::Sprite sprite;
    b2World world;
};