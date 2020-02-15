#include "robot.hpp"

#include <fstream>
#include <iostream>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <json.hpp>

#include "constants.hpp"
#include "environment.hpp"

Robot::Robot(Environment& env, int x, int y) :
    env{env},
    field{env.getField()},
    origin{env.getField().getOrigin()},
    texture{},
    sprite{} {
    texture.loadFromFile(ROBOT_IMAGE_NAME);

    float xSize = getPixelSize().x;
    float ySize = getPixelSize().y;

    physicalSize = sf::Vector2f{field.p2mX(xSize), field.p2mY(ySize)};

    std::ifstream file{ROBOT_METADATA_NAME};
    nlohmann::json metadata;
    file >> metadata;

    metadata["trackWidthMeter"].get_to(trackWidth);

    sprite.setTexture(texture);
    sprite.setOrigin(xSize / 2, ySize / 2);
    createRobot(x, y);
}

sf::Vector2u Robot::getPixelSize() const {
    return texture.getSize();
}

sf::Vector2f Robot::getPhysicalSize() const {
    return physicalSize;
}

float Robot::getTrackWidth() const {
    return trackWidth;
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
    angularSpeed = (right - left) / trackWidth;
}

void Robot::stop() {
    linearSpeed = 0.0f;
    angularSpeed = 0.0f;
}

ChassisSpeeds Robot::getChassisSpeeds() {
    return std::make_pair(linearSpeed, angularSpeed);
}

WheelSpeeds Robot::getWheelSpeeds() {
    return std::make_pair(linearSpeed - (trackWidth / 2) * angularSpeed,
        linearSpeed + (trackWidth / 2) * angularSpeed);
}

void Robot::setMeterPosition(float x, float y, float theta) {
    body->SetTransform(b2Vec2{x, y}, theta);
}

void Robot::setPixelPosition(int x, int y, float theta) {
    body->SetTransform(b2Vec2{
        field.p2mX(x - origin.x), field.p2mY(origin.y - y)
    }, theta);
}

void Robot::setAngle(float theta) {
    body->SetTransform(body->GetPosition(), theta);
}

void Robot::update(float dt) {
    // Apply impulses depending on the chassis speed 
    float currentAngle = body->GetAngle();

    // Target velocity
    const b2Vec2 targetV = b2Vec2{
        linearSpeed * std::cos(currentAngle), linearSpeed * std::sin(currentAngle)
    };
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
    sprite.setPosition(
        origin.x + field.m2pX(body->GetPosition().x), 
        origin.y + MENU_BAR_HEIGHT - field.m2pY(body->GetPosition().y)
    );
    // SFML +angle = clockwise, Box2D +angle = counter-clockwise
    sprite.setRotation(-body->GetAngle() * 180 / b2_pi);
    window.draw(sprite);
}

void Robot::createRobot(int x, int y) {
    b2BodyDef bodyDef;
    bodyDef.position = b2Vec2(field.p2mX(x - origin.x), field.p2mY(origin.y - y));
    
    bodyDef.type = b2_dynamicBody;
    body = env.getWorld().CreateBody(&bodyDef);

    b2PolygonShape shape;
    shape.SetAsBox(physicalSize.x / 2, physicalSize.y / 2);

    b2FixtureDef fixtureDef;
    fixtureDef.density = 100.0f;
    //fixtureDef.friction = 0.7f;
    fixtureDef.shape = &shape;
    body->CreateFixture(&fixtureDef);
}