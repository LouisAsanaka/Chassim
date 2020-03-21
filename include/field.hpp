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
    const sf::Vector2i& getSpawnPoint() const;
    const std::vector<std::vector<b2Vec2>>& getWalls() const;
    const std::vector<std::vector<b2Vec2>>& getPolygons() const;
    bool isInField(int x, int y);

    const inline float p2mX(float coord) const {
        return coord / fieldXPixelPerMeter;
    }

    const inline float p2mY(float coord) const {
        return coord / fieldYPixelPerMeter;
    }

    const inline float m2pX(float coord) const {
        return coord * fieldXPixelPerMeter;
    }

    const inline float m2pY(float coord) const {
        return coord * fieldYPixelPerMeter;
    }
private:
    void loadImage();
    void loadData();

    sf::Texture texture;
    
    int fieldPixelWidth;
    float fieldMeterWidth;
    float fieldXPixelPerMeter;

    int fieldPixelHeight;
    float fieldMeterHeight;
    float fieldYPixelPerMeter;

    sf::Vector2i spawnPoint;
    std::vector<std::vector<b2Vec2>> walls;
    std::vector<std::vector<b2Vec2>> polygons;
};