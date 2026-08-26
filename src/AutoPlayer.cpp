#include "AutoPlayer.hpp"
#include <algorithm>
#include <limits>

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

    auto none = m_predictor.evaluate(state, Action::None, horizon);
    auto press = m_predictor.evaluate(state, Action::Press, horizon);
    auto release = m_predictor.evaluate(state, Action::Release, horizon);

    Candidate* best = &none;
    if (!press.unsafe && press.score < best->score) best = &press;
    if (!release.unsafe && release.score < best->score) best = &release;

    // Avoid acting on a weak prediction.
    float second = std::numeric_limits<float>::max();
    for (auto* c : {&none, &press, &release}) {
        if (c != best) second = std::min(second, c->score);
    }

    float gap = second - best->score;
    m_confidence = gap <= 0.f ? 0.f : std::min(1.f, gap / (std::abs(second) + 1.f));

    m_action = m_confidence < 0.10f ? Action::None : best->action;
}
