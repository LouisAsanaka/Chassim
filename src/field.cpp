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

    auto& wallsData = metadata["walls"];
    for (auto& wallData : wallsData) {
        walls.emplace_back(
            std::initializer_list<b2Vec2>{
                b2Vec2{
                    p2m(wallData[0][0].get<float>()),
                    p2m(wallData[0][1].get<float>())
                },
                b2Vec2{
                    p2m(wallData[1][0].get<float>()),
                    p2m(wallData[1][1].get<float>())
                },
            }
        );
    }

    auto& polysData = metadata["polygons"];
    for (auto& polyData : polysData) {
        std::vector<b2Vec2> verticies;
        for (auto& polyCoords : polyData) {
            verticies.emplace_back(
                p2m(polyCoords[0].get<float>()), p2m(polyCoords[1].get<float>()));
        }
        polygons.push_back(std::move(verticies));
    }
}
