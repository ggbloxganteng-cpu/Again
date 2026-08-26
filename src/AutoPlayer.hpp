#pragma once
#include "Predictor.hpp"

class AutoPlayer {
public:
    void setEnabled(bool value);
    bool enabled() const;

    void update(RuntimeState const& state, int horizon);
    Action action() const;
    float confidence() const;

private:
    bool m_enabled = false;
    Action m_action = Action::None;
    float m_confidence = 0.f;
    Predictor m_predictor;
};
