#include "robot.hpp"

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "constants.hpp"
#include "environment.hpp"

Robot::Robot(Environment& env, int x, int y) :
    env{env},
    field{env.getField()},
    physicalSize{env.getField().p2m(ROBOT_SPRITE_SIZE)},
    halfTrackWidth{physicalSize / 2},
    texture{},
    sprite{} {
    texture.loadFromFile("asset/robot.png");
    sprite.setTexture(texture);
    sprite.setOrigin(ROBOT_SPRITE_SIZE / 2, ROBOT_SPRITE_SIZE / 2);
    createRobot(x, y);
}

float Robot::getPhysicalSize() {
    return physicalSize;
}

b2Body* Robot::getBody() {
    return body;
}

void Robot::setChassisSpeeds(float linear, float angular) {
    linearSpeed = linear;
    angularSpeed = angular;
}

void Robot::setWheelSpeeds(float left, float right) {
    linearSpeed = (left + right) / 2;
    angularSpeed = (left - right) / physicalSize;
}

void Robot::stop() {
    linearSpeed = 0.0f;
    angularSpeed = 0.0f;
}

ChassisSpeeds Robot::getChassisSpeeds() {
    return std::make_pair(linearSpeed, angularSpeed);
}

WheelSpeeds Robot::getWheelSpeeds() {
    return std::make_pair(linearSpeed - halfTrackWidth * angularSpeed,
        linearSpeed + halfTrackWidth * angularSpeed);
}

void Robot::setPosition(int x, int y, float theta) {
    body->SetTransform(b2Vec2{field.p2m(x), field.p2m(y)}, theta);
}

void Robot::update() {
    // Apply impulses depending on the chassis speed 
    float currentAngle = body->GetAngle();

    // Target velocity
    const b2Vec2 targetV = b2Vec2{
        linearSpeed * std::cos(currentAngle), linearSpeed * std::sin(currentAngle)};
    const b2Vec2& currentV = body->GetLinearVelocity();
    b2Vec2 dv = targetV - currentV;

    b2Vec2 linearImpulse = body->GetMass() * dv;
    body->ApplyLinearImpulse(linearImpulse, body->GetWorldCenter(), true);

    // Change in angular velocity (omega)
    float dw = angularSpeed - body->GetAngularVelocity();
    float angularImpulse = body->GetInertia() * dw;
    body->ApplyAngularImpulse(angularImpulse, true);
}

void Robot::render(sf::RenderWindow& window) {
    sprite.setPosition(field.m2p(body->GetPosition().x), field.m2p(body->GetPosition().y) + MENU_BAR_HEIGHT);
    sprite.setRotation(body->GetAngle() * 180 / b2_pi);
    window.draw(sprite);
}

void Robot::createRobot(int x, int y) {
    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2(field.p2m(x), field.p2m(y));
    bodyDef.type = b2_dynamicBody;
    body = env.getWorld().CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(physicalSize / 2, physicalSize / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.density = 100.0f;
    //fixtureDef.friction = 0.7f;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}