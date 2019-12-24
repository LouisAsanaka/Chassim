#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#define FIELD_IMAGE_NAME "asset/field.png"
#define FIELD_METADATA_NAME "asset/field.json"

class Field {
public:
    Field();

    const sf::Texture& getTexture();
    const std::vector<std::vector<b2Vec2>>& getWalls();
    const std::vector<std::vector<b2Vec2>>& getPolygons();
private:
    void loadImage();
    void loadData();

    sf::Texture texture;
    std::vector<std::vector<b2Vec2>> walls;
    std::vector<std::vector<b2Vec2>> polygons;
};