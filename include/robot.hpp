#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "field.hpp"
#include "environment.hpp"

#define ROBOT_SPRITE_SIZE 64.0f

typedef std::pair<float, float> ChassisSpeeds;
typedef std::pair<float, float> WheelSpeeds;

class Robot {
public:
    Robot(Environment& env, int x, int y);

    float getPhysicalSize();

    b2Body* getBody();
    void setChassisSpeeds(float linear, float angular);
    void setWheelSpeeds(float left, float right);
    void stop();
    ChassisSpeeds getChassisSpeeds();
    WheelSpeeds getWheelSpeeds();

    void setPosition(int x, int y, float theta = 0.0f);

    void update();
    void render(sf::RenderWindow& window);
private:
    void createRobot(int x, int y);

    float physicalSize;
    float halfTrackWidth;

    sf::Texture texture;
    sf::Sprite sprite;
    b2Body* body;
    Environment& env;
    Field& field;

    float linearSpeed = 0.0f;
    float angularSpeed = 0.0f;
};