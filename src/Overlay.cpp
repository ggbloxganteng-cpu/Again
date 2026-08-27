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

// ─── BUG #1 FIXED ───────────────────────────────────────────────────────────
// LAMA (infinite recursion):   this->clear();
// BARU (panggil parent):       CCDrawNode::clear();
void Overlay::clear() {
    CCDrawNode::clear();
}

void Overlay::drawTrajectory(Trajectory const& t, TrajectoryStatus status) {
    CCDrawNode::clear();   // bersihkan frame sebelumnya

    auto const& pts = t.points();
    if (pts.size() < 2) return;

    // ─── BUG #2 FIXED ───────────────────────────────────────────────────────
    // LAMA: drawSegment(from, to, radius)          ← missing color → compile error
    // BARU: drawSegment(from, to, radius, color)   ← correct ccColor4F
    ccColor4F color;
    switch (status) {
        case TrajectoryStatus::Danger:
            color = {1.f, 0.2f, 0.2f, 0.85f}; break;   // Merah  — bahaya
        case TrajectoryStatus::Press:
            color = {0.2f, 1.f, 0.35f, 0.85f}; break;  // Hijau  — sedang press
        case TrajectoryStatus::Release:
            color = {1.f, 0.9f, 0.1f, 0.85f}; break;   // Kuning — sedang release
        default: /* Safe */
            color = {0.4f, 0.9f, 1.f, 0.75f}; break;   // Cyan   — aman
    }

    for (size_t i = 1; i < pts.size(); ++i) {
        this->drawSegment(pts[i - 1].pos, pts[i].pos, 1.5f, color);
    }

    // Titik awal (posisi player saat ini)
    this->drawDot(pts.front().pos, 3.5f, color);
}
