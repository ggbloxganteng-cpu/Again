#include "RuntimeState.hpp"

RuntimeState RuntimeStateReader::read(PlayerObject* p) {
    RuntimeState s;
    if (!p) return s;

    s.position = p->getPosition();
    s.yVelocity = static_cast<float>(p->m_yVelocity);
    s.xVelocity = p->m_platformerXVelocity != 0.0
        ? static_cast<float>(p->m_platformerXVelocity)
        : p->m_xVelocityRelated;
    s.speed = p->m_playerSpeed;
    s.gravity = p->m_gravityMod;
    s.holding = p->m_holdingButtons.empty() ? false : p->m_holdingButtons.begin()->second;
    s.dead = p->m_isDead;
    s.onGround = p->m_isOnGround;
    s.ship = p->m_isShip;
    s.ball = p->m_isBall;
    s.wave = p->m_isDart;
    s.robot = p->m_isRobot;
    s.spider = p->m_isSpider;
    s.platformer = p->m_isPlatformer;
    return s;
}
