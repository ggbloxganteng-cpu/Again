#pragma once
#include "RuntimeState.hpp"
#include "Trajectory.hpp"

enum class Action {
    None,
    Press,
    Release
};

struct Candidate {
    Action action = Action::None;
    float score = 0.f;
    bool unsafe = false;
    Trajectory path;
};

class Predictor {
public:
    Candidate evaluate(RuntimeState const& state, Action action, int frames) const;

private:
    RuntimeState step(RuntimeState state, Action action) const;
    float dangerScore(RuntimeState const& state) const;
};
