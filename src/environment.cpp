#include "environment.hpp"
#include "utils.hpp"
#include <numeric>

Environment::Environment(const sf::Texture& texture) :
    world{b2Vec2{0.0f, 0.0f}},
    texture{texture},
    sprite{} {

    sprite.setTexture(texture);
    sprite.setPosition(0.0f, MENU_BAR_HEIGHT);

    createBoundary(52, 32, 1367, 39);  // Top wall
    createBoundary(1367, 40, 1374, 691); // Right wall
    createBoundary(52, 692, 1368, 699);  // Bottom wall
    createBoundary(44, 39, 51, 690);   // Left wall
    createBoundary(496, 307, 923, 421);   // Cargo ship

    b2Vec2 topLeft[] = {
        b2Vec2{475, 39},
        b2Vec2{475, 57},
        b2Vec2{496, 95},
        b2Vec2{534, 95},
        b2Vec2{555, 57},
        b2Vec2{555, 39}
    };
    P2M_VECS(topLeft);
    createRocket(topLeft, 6);

    b2Vec2 topRight[] = {
        b2Vec2{864, 39},
        b2Vec2{864, 57},
        b2Vec2{885, 95},
        b2Vec2{923, 95},
        b2Vec2{945, 57},
        b2Vec2{945, 39}
    };
    P2M_VECS(topRight);
    createRocket(topRight, 6);

    b2Vec2 bottomLeft[] = {
        b2Vec2{475, 691},
        b2Vec2{475, 672},
        b2Vec2{496, 635},
        b2Vec2{534, 635},
        b2Vec2{555, 672},
        b2Vec2{555, 691}
    };
    P2M_VECS(bottomLeft);
    createRocket(bottomLeft, 6);

    b2Vec2 bottomRight[] = {
        b2Vec2{864, 691},
        b2Vec2{864, 672},
        b2Vec2{885, 635},
        b2Vec2{923, 635},
        b2Vec2{945, 672},
        b2Vec2{945, 691}
    };
    P2M_VECS(bottomRight);
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

