#include "RuntimeState.hpp"

RuntimeState RuntimeStateReader::read(PlayerObject* p) {
    RuntimeState s;
    if (!p) return s;

    s.position  = p->getPosition();
    s.yVelocity = static_cast<float>(p->m_yVelocity);

    // ─── BUG FIXED ──────────────────────────────────────────────────────────
    // LAMA: p->m_xVelocityRelated   ← field ini tidak ada di GD 2.2081 bindings
    // BARU:
    //   - Platformer mode  → gunakan m_platformerXVelocity (tersedia di binding)
    //   - Classic mode     → gunakan m_playerSpeed (unit: game-units/frame)
    //
    // Catatan Phase 2: perlu kalibrasi unit m_playerSpeed vs unit physic engine.
    // Saat ini speed 0.9 (normal) ≈ 8–10 unit/frame; m_playerSpeed sudah dalam
    // unit yang sama sehingga bisa dipakai langsung sebagai xVelocity.
    if (p->m_isPlatformer) {
        s.xVelocity = static_cast<float>(p->m_platformerXVelocity);
    } else {
        s.xVelocity = p->m_playerSpeed;   // classic mode: speed = x vel per frame
    }

    s.speed   = p->m_playerSpeed;
    s.gravity = p->m_gravityMod;

    // m_holdingButtons = gd::map<int, bool> (button 1 = jump)
    {
        auto it = p->m_holdingButtons.find(1);
        s.holding = (it != p->m_holdingButtons.end()) && it->second;
    }

    s.dead      = p->m_isDead;
    s.onGround  = p->m_isOnGround;
    s.ship      = p->m_isShip;
    s.ball      = p->m_isBall;
    s.wave      = p->m_isDart;
    s.robot     = p->m_isRobot;
    s.spider    = p->m_isSpider;
    s.platformer= p->m_isPlatformer;
    return s;
}
