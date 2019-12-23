#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "environment.hpp"
#include "utils.hpp"

#define ROBOT_SPRITE_SIZE 64.0f
#define ROBOT_PHYSICAL_SIZE (P2M(ROBOT_SPRITE_SIZE))
#define HALF_TRACKWIDTH ROBOT_PHYSICAL_SIZE / 2

typedef std::pair<float, float> ChassisSpeeds;
typedef std::pair<float, float> WheelSpeeds;

class Robot {
public:
    Robot(Environment& env, int x, int y);

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

    sf::Texture texture;
    sf::Sprite sprite;
    b2Body* body;
    Environment& env;

    float linearSpeed = 0.0f;
    float angularSpeed = 0.0f;
};