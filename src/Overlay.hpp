#pragma once
#include "Trajectory.hpp"

class Overlay : public cocos2d::CCDrawNode {
public:
    static Overlay* create();
    void drawTrajectory(Trajectory const& t, cocos2d::CCNode* parent);
    void clear();
};
