#pragma once
#include "Trajectory.hpp"

// Status warna trajectory:
//   Safe    = Cyan   — jalur aman, tidak ada collision
//   Danger  = Red    — prediksi akan menabrak
//   Press   = Green  — bot sedang/akan menekan jump
//   Release = Yellow — bot sedang/akan melepas jump
enum class TrajectoryStatus {
    Safe,
    Danger,
    Press,
    Release
};

class Overlay : public cocos2d::CCDrawNode {
public:
    static Overlay* create();

    // Gambar trajectory ke overlay; panggil setiap frame dari update()
    void drawTrajectory(Trajectory const& t,
                        TrajectoryStatus  status = TrajectoryStatus::Safe);

    // Hapus semua gambar — BUKAN this->clear() (itu rekursif!)
    void clear();

private:
    bool init() { return CCDrawNode::init(); }
};
