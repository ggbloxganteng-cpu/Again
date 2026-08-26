#include "Trajectory.hpp"

void Trajectory::clear() { m_points.clear(); }

void Trajectory::add(cocos2d::CCPoint p, int frame) {
    m_points.push_back({p, frame});
}

std::vector<TrajectoryPoint> const& Trajectory::points() const {
    return m_points;
}
