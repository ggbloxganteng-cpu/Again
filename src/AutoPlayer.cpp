#include "AutoPlayer.hpp"
#include <algorithm>
#include <cmath>

void AutoPlayer::setEnabled(bool value) {
    m_enabled = value;
    if (!value) {
        m_action = Action::None;
        m_confidence = 0.f;
    }
}

bool AutoPlayer::enabled() const { return m_enabled; }

void AutoPlayer::update(RuntimeState const& state, int horizon) {
    if (!m_enabled || state.dead) {
        m_action = Action::None;
        m_confidence = 0.f;
        return;
    }

    // Phase-1 Android fallback: the old predictor always selected None because
    // it had no collision information. That meant the bot never generated an
    // input. For cube/robot/spider, perform a real jump pulse when grounded.
    // Other modes continue to use the predictor until their mode-specific
    // controller is implemented.
    if (!state.ship && !state.wave && !state.ball) {
        if (state.onGround) {
            m_action = Action::Press;
            m_confidence = 1.f;
            return;
        }

        // Release after the initial jump pulse. This gives the input layer a
        // deterministic press/release pair instead of an endless hold.
        m_action = Action::Release;
        m_confidence = 0.65f;
        return;
    }

    auto none = m_predictor.evaluate(state, Action::None, horizon);
    auto press = m_predictor.evaluate(state, Action::Press, horizon);
    auto release = m_predictor.evaluate(state, Action::Release, horizon);

    Candidate* best = &none;
    if (!press.unsafe && press.score < best->score) best = &press;
    if (!release.unsafe && release.score < best->score) best = &release;

    float second = std::numeric_limits<float>::max();
    for (auto* c : {&none, &press, &release}) {
        if (c != best) second = std::min(second, c->score);
    }

    float gap = second - best->score;
    m_confidence = gap <= 0.f ? 0.f : std::min(1.f, gap / (std::abs(second) + 1.f));
    m_action = m_confidence < 0.05f ? Action::None : best->action;
}
