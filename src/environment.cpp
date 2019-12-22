#include "environment.hpp"
#include "utils.hpp"
#include <numeric>

Environment::Environment() :
    world{b2Vec2{0.0f, 0.0f}},
    texture{},
    sprite{} {

    texture.loadFromFile("asset/field.png");
    sprite.setTexture(texture);
    sprite.setPosition(0.0f, 0.0f);

    createBoundary(242, 32, 1557, 39);  // Top wall
    createBoundary(1557, 40, 1564, 691); // Right wall
    createBoundary(242, 692, 1558, 699);  // Bottom wall
    createBoundary(234, 39, 241, 690);   // Left wall
    createBoundary(686, 307, 1113, 421);   // Cargo ship

    b2Vec2 topLeft[] = {
        b2Vec2{665, 39},
        b2Vec2{665, 57},
        b2Vec2{686, 95},
        b2Vec2{724, 95},
        b2Vec2{745, 57},
        b2Vec2{745, 39}
    };
    P2M_VEC(topLeft);
    createRocket(topLeft, 6);

    b2Vec2 topRight[] = {
        b2Vec2{1054, 39},
        b2Vec2{1054, 57},
        b2Vec2{1075, 95},
        b2Vec2{1113, 95},
        b2Vec2{1135, 57},
        b2Vec2{1135, 39}
    };
    P2M_VEC(topRight);
    createRocket(topRight, 6);

    b2Vec2 bottomLeft[] = {
        b2Vec2{665, 691},
        b2Vec2{665, 672},
        b2Vec2{686, 635},
        b2Vec2{724, 635},
        b2Vec2{745, 672},
        b2Vec2{745, 691}
    };
    P2M_VEC(bottomLeft);
    createRocket(bottomLeft, 6);

    b2Vec2 bottomRight[] = {
        b2Vec2{1054, 691},
        b2Vec2{1054, 672},
        b2Vec2{1075, 635},
        b2Vec2{1113, 635},
        b2Vec2{1135, 672},
        b2Vec2{1135, 691}
    };
    P2M_VEC(bottomRight);
    createRocket(bottomRight, 6);
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

void Environment::createBoundary(int startX, int startY, int endX, int endY) {
    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2(
        P2M((startX + endX) / 2.0f),
        P2M((startY + endY) / 2.0f)
    );
    bodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(P2M(std::abs(endX - startX) / 2), P2M(std::abs(endY - startY) / 2));

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}

void Environment::createRocket(const b2Vec2* points, int length) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    b2Body* body = world.CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.Set(points, length);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}

