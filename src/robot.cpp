#include "robot.hpp"
#include "environment.hpp"
#include "utils.hpp"
#include <iostream>

Robot::Robot(Environment& env, int x, int y) :
    env{env}, 
    texture{},
    sprite{} {
    texture.loadFromFile("asset/robot.png");
    sprite.setTexture(texture);
    sprite.setOrigin(ROBOT_SPRITE_SIZE / 2, ROBOT_SPRITE_SIZE / 2);
    createRobot(x, y);
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
    angularSpeed = (left - right) / ROBOT_PHYSICAL_SIZE;
}

ChassisSpeeds Robot::getChassisSpeeds() {
    return std::make_pair(linearSpeed, angularSpeed);
}

WheelSpeeds Robot::getWheelSpeeds() {
    return std::make_pair(linearSpeed - HALF_TRACKWIDTH * angularSpeed,
        linearSpeed + HALF_TRACKWIDTH * angularSpeed);
}

void Robot::setPosition(int x, int y) {
    body->SetTransform(b2Vec2{P2M(x), P2M(y)}, 0.0f);
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
    sprite.setPosition(M2P(body->GetPosition().x), M2P(body->GetPosition().y));
    sprite.setRotation(body->GetAngle() * 180 / b2_pi);
    window.draw(sprite);
}

void Robot::createRobot(int x, int y) {
    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2(P2M(x), P2M(y));
    bodyDef.type = b2_dynamicBody;
    body = env.getWorld().CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(P2M(ROBOT_SPRITE_SIZE / 2), P2M(ROBOT_SPRITE_SIZE / 2));

    b2FixtureDef fixtureDef;
    fixtureDef.density = 100.0f;
    //fixtureDef.friction = 0.7f;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}