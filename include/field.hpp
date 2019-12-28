#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#define FIELD_IMAGE_NAME "asset/field.png"
#define FIELD_METADATA_NAME "asset/field.json"

class Field {
public:
    Field();

    const sf::Texture& getTexture() const;
    const int getPixelsPerMeter() const;
    const sf::Vector2f& getOrigin() const;
    const sf::Vector2i& getSpawnPoint() const;
    const std::vector<std::vector<b2Vec2>>& getWalls() const;
    const std::vector<std::vector<b2Vec2>>& getPolygons() const;

    const inline float p2m(float coord) const {
        return coord / pixelsPerMeter;
    }

    const inline float m2p(float coord) const {
        return coord * pixelsPerMeter;
    }
private:
    void loadImage();
    void loadData();

    sf::Texture texture;
    sf::Vector2f physicalSize;
    int pixelsPerMeter;
    sf::Vector2f origin;
    sf::Vector2i spawnPoint;
    std::vector<std::vector<b2Vec2>> walls;
    std::vector<std::vector<b2Vec2>> polygons;
};