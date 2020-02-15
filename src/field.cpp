#include "field.hpp"

#include <vector>
#include <initializer_list>
#include <fstream>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include "json.hpp"

Field::Field() :
    texture{}, walls{}, polygons{} {
    loadImage();
    loadData();
}

const sf::Texture& Field::getTexture() const {
    return texture;
}

const sf::Vector2i& Field::getSpawnPoint() const {
    return spawnPoint;
}

const std::vector<std::vector<b2Vec2>>& Field::getWalls() const {
    return walls;
}

const std::vector<std::vector<b2Vec2>>& Field::getPolygons() const {
    return polygons;
}

void Field::loadImage() {
    texture.loadFromFile(FIELD_IMAGE_NAME);
}

void Field::loadData() {
    std::ifstream file{FIELD_METADATA_NAME};
    nlohmann::json metadata;
    file >> metadata;

    metadata["fieldPixelWidth"].get_to(fieldPixelWidth);
    metadata["fieldMeterWidth"].get_to(fieldMeterWidth);
    fieldXPixelPerMeter = fieldPixelWidth / fieldMeterWidth;

    metadata["fieldPixelHeight"].get_to(fieldPixelHeight);
    metadata["fieldMeterHeight"].get_to(fieldMeterHeight);
    fieldYPixelPerMeter = fieldPixelHeight / fieldMeterHeight;

    spawnPoint = sf::Vector2i{
        metadata["spawnPoint"][0].get<int>(), metadata["spawnPoint"][1].get<int>()
    };

    auto& wallsData = metadata["walls"];
    for (auto& wallData : wallsData) {
        walls.emplace_back(
            std::initializer_list<b2Vec2>{
                b2Vec2{
                    p2mX(wallData[0][0].get<float>()),
                    p2mY(wallData[0][1].get<float>())
                },
                b2Vec2{
                    p2mX(wallData[1][0].get<float>()),
                    p2mY(wallData[1][1].get<float>())
                },
            }
        );
    }

    auto& polysData = metadata["polygons"];
    for (auto& polyData : polysData) {
        std::vector<b2Vec2> verticies;
        for (auto& polyCoords : polyData) {
            verticies.emplace_back(
                p2mX(polyCoords[0].get<float>()), p2mY(polyCoords[1].get<float>()));
        }
        polygons.push_back(std::move(verticies));
    }
}
