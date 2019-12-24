#include "environment.hpp"

#include <vector>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "field.hpp"
#include "utils.hpp"

Environment::Environment(Field& field) :
    world{b2Vec2{0.0f, 0.0f}},
    texture{field.getTexture()},
    sprite{} {

    sprite.setTexture(texture);
    sprite.setPosition(0.0f, MENU_BAR_HEIGHT);

    for (auto& wall : field.getWalls()) {
        createWall(wall);
    }
    for (auto& polygon : field.getPolygons()) {
        createPolygon(polygon);
    }
}

b2World& Environment::getWorld() {
    return world;
}

void Environment::update() {
    world.Step(1 / 60.0f, 8, 3);
}

void Environment::render(sf::RenderWindow& window) {
    window.draw(sprite);
}

void Environment::createWall(const std::vector<b2Vec2>& vec) {
    float startX = vec[0].x;
    float endX = vec[1].x;
    float startY = vec[0].y;
    float endY = vec[1].y;

    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2(
        (startX + endX) / 2.0f,
        (startY + endY) / 2.0f
    );
    bodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(std::abs(endX - startX) / 2, std::abs(endY - startY) / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}

void Environment::createPolygon(const std::vector<b2Vec2>& vec) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.Set(vec.data(), vec.size());

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}

