#include "Predictor.hpp"
#include <cmath>

RuntimeState Predictor::step(RuntimeState s, Action action) const {
    bool press = action == Action::Press;
    s.holding = press;

    // Conservative local approximation. This is intentionally not
    // pretending to be the complete GD physics engine.
    const float g = 0.72f * s.gravity;
    const float jump = 2.55f;

    if (press && s.onGround)
        s.yVelocity = jump * (s.gravity >= 0.f ? 1.f : -1.f);

    if (!s.ship && !s.wave && !s.ball)
        s.yVelocity += g;

    s.position.x += s.xVelocity;
    s.position.y += s.yVelocity;
    s.onGround = false;
    return s;
}

float Predictor::dangerScore(RuntimeState const& s) const {
    // The object/collision layer can later add exact distances.
    // Keep the score finite and deterministic.
    if (std::abs(s.position.y) > 5000.f)
        return 100000.f;
    return std::abs(s.yVelocity) * 0.02f;
}

Candidate Predictor::evaluate(RuntimeState const& initial, Action action, int frames) const {
    Candidate c;
    c.action = action;

    RuntimeState s = initial;
    float score = 0.f;

    for (int i = 0; i < frames; ++i) {
        s = step(s, action);
        score += dangerScore(s);
        c.path.add(s.position, i);
        if (s.dead) {
            c.unsafe = true;
            score += 100000.f;
            break;
        }
    }

    c.score = score;
    return c;
}
