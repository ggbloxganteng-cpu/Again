#pragma once
#include <Geode/Geode.hpp>
#include <vector>

struct TrajectoryPoint {
    cocos2d::CCPoint pos{};
    int frame = 0;
};

class Trajectory {
public:
    void clear();
    void add(cocos2d::CCPoint p, int frame);
    std::vector<TrajectoryPoint> const& points() const;

private:
    std::vector<TrajectoryPoint> m_points;
};
