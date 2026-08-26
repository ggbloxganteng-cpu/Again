#include "Overlay.hpp"

using namespace cocos2d;

Overlay* Overlay::create() {
    auto p = new Overlay();

    if (p && p->init()) {
        p->autorelease();
        return p;
    }

    CC_SAFE_DELETE(p);
    return nullptr;
}

void Overlay::clear() {
    CCDrawNode::clear();
}

void Overlay::drawTrajectory(Trajectory const& t, CCNode* parent) {
    if (!parent)
        return;

    this->clear();

    auto const& pts = t.points();

    if (pts.size() < 2)
        return;

    ccColor4F color = {
        0.f,
        1.f,
        0.f,
        0.8f
    };

    for (size_t i = 1; i < pts.size(); ++i) {
        this->drawSegment(
            pts[i - 1].pos,
            pts[i].pos,
            1.f,
            color
        );
    }
}
