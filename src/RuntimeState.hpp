#pragma once
#include <Geode/Geode.hpp>

struct RuntimeState {
    cocos2d::CCPoint position{};
    float yVelocity = 0.f;
    float xVelocity = 0.f;
    float speed = 0.f;
    float gravity = 1.f;
    bool holding = false;
    bool dead = false;
    bool onGround = false;
    bool ship = false;
    bool ball = false;
    bool wave = false;
    bool robot = false;
    bool spider = false;
    bool platformer = false;
};

class RuntimeStateReader {
public:
    static RuntimeState read(PlayerObject* player);
};
