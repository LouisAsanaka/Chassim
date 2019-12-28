#include "field.hpp"

#include <vector>
#include <initializer_list>
#include <fstream>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <json.hpp>

Field::Field() :
    texture{}, walls{}, polygons{} {
    loadImage();
    loadData();

    physicalSize.x = p2m(texture.getSize().x);
    physicalSize.y = p2m(texture.getSize().y);
}

const sf::Texture& Field::getTexture() const {
    return texture;
}

const sf::Vector2f& Field::getOrigin() const {
    return origin;
}

const sf::Vector2i& Field::getSpawnPoint() const {
    return spawnPoint;
}

const int Field::getPixelsPerMeter() const {
    return pixelsPerMeter;
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

    metadata["pixelsPerMeter"].get_to(pixelsPerMeter);
    origin = sf::Vector2f{
        metadata["origin"][0].get<float>(), metadata["origin"][1].get<float>()
    };
    spawnPoint = sf::Vector2i{
        metadata["spawnPoint"][0].get<int>(), metadata["spawnPoint"][1].get<int>()
    };

    auto& wallsData = metadata["walls"];
    for (auto& wallData : wallsData) {
        walls.emplace_back(
            std::initializer_list<b2Vec2>{
                b2Vec2{
                    p2m(wallData[0][0].get<float>() - origin.x),
                    p2m(origin.y - wallData[0][1].get<float>())
                },
                b2Vec2{
                    p2m(wallData[1][0].get<float>() - origin.x),
                    p2m(origin.y - wallData[1][1].get<float>())
                },
            }
        );
    }

    auto& polysData = metadata["polygons"];
    for (auto& polyData : polysData) {
        std::vector<b2Vec2> verticies;
        for (auto& polyCoords : polyData) {
            verticies.emplace_back(
                p2m(polyCoords[0].get<float>() - origin.x), 
                p2m(origin.y - polyCoords[1].get<float>())
            );
        }
        polygons.push_back(std::move(verticies));
    }
}
