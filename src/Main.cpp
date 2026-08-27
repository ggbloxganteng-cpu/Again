// TrajectoryBot — Phase 1
// PlayLayer hook lengkap: init, update, resetLevel, destroyPlayer
// Overlay sekarang benar-benar ditambahkan ke scene dan digambar setiap frame.
// AutoPlayer sekarang benar-benar mengirim input via handleButton().

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "RuntimeState.hpp"
#include "AutoPlayer.hpp"
#include "Predictor.hpp"
#include "Overlay.hpp"

using namespace geode::prelude;

class $modify(TrajectoryBotPlayLayer, PlayLayer) {

    // ── Per-instance state (m_fields pattern — aman untuk multiple instances) ──
    struct Fields {
        Ref<Overlay> m_overlay   = nullptr;   // smart-ptr, auto-release safe
        AutoPlayer   m_bot;
        Action       m_lastAction = Action::None;
    };

    // ═══════════════════════════════════════════════════════════════════════
    // INIT
    // ═══════════════════════════════════════════════════════════════════════
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        // ─── BUG FIXED: overlay sekarang DIBUAT dan DITAMBAH ke scene ────────
        // Sebelumnya: Overlay tidak pernah di-create → tidak ada yang tergambar.
        // Sekarang: di-create saat PlayLayer::init() dan ditambah ke m_gameLayer.
        //
        // Kenapa m_gameLayer?
        //   Posisi player dari getPosition() = koordinat di m_gameLayer.
        //   Dengan menambah overlay ke m_gameLayer, titik trajectory langsung
        //   sejajar dengan posisi player tanpa perlu manual offset kamera.
        auto* ov = Overlay::create();
        if (ov) {
            ov->setPosition(CCPointZero);    // origin = sama dengan m_gameLayer
            ov->setAnchorPoint(CCPointZero);
            ov->setZOrder(999);              // di atas semua objek level

            CCNode* target = this->m_gameLayer
                           ? static_cast<CCNode*>(this->m_gameLayer)
                           : static_cast<CCNode*>(this);
            target->addChild(ov);
            m_fields->m_overlay = ov;

            log::debug("[TrajectoryBot] Overlay attached to {}",
                this->m_gameLayer ? "m_gameLayer" : "PlayLayer (fallback)");
        } else {
            log::warn("[TrajectoryBot] Overlay::create() gagal — cek init CCDrawNode");
        }

        // Sync bot dari setting
        m_fields->m_bot.setEnabled(
            Mod::get()->getSettingValue<bool>("bot-enabled")
        );

        return true;
    }

    // ═══════════════════════════════════════════════════════════════════════
    // UPDATE — dipanggil setiap frame
    // ═══════════════════════════════════════════════════════════════════════
    void update(float dt) {
        PlayLayer::update(dt);

        auto* ov = m_fields->m_overlay;
        if (!ov) return;

        auto* player = this->m_player1;
        if (!player || player->m_isDead) {
            ov->clear();
            return;
        }

        const bool trajOn = Mod::get()->getSettingValue<bool>("trajectory-enabled");
        const bool botOn  = Mod::get()->getSettingValue<bool>("bot-enabled");

        RuntimeState state = RuntimeStateReader::read(player);
        Predictor    predictor;

        // ── TRAJECTORY OVERLAY ────────────────────────────────────────────
        // BUG FIXED: drawTrajectory() sekarang dipanggil setiap frame.
        // Sebelumnya: tidak pernah dipanggil → overlay selalu kosong.
        if (trajOn) {
            int frames = Mod::get()->getSettingValue<int>("prediction-frames");

            // Visualisasikan path yang sedang diikuti bot (atau path None jika bot off)
            Action vizAction = (botOn && m_fields->m_lastAction != Action::None)
                ? m_fields->m_lastAction : Action::None;

            auto cand = predictor.evaluate(state, vizAction, frames);

            TrajectoryStatus tStatus;
            if      (cand.unsafe)                       tStatus = TrajectoryStatus::Danger;
            else if (vizAction == Action::Press)        tStatus = TrajectoryStatus::Press;
            else if (vizAction == Action::Release)      tStatus = TrajectoryStatus::Release;
            else                                        tStatus = TrajectoryStatus::Safe;

            ov->drawTrajectory(cand.path, tStatus);
        } else {
            ov->clear();
        }

        // ── AUTO PLAYER ───────────────────────────────────────────────────
        // BUG FIXED: handleButton() sekarang benar-benar dipanggil.
        // Sebelumnya: bot hanya menghitung action tapi tidak pernah kirim input.
        if (botOn) {
            auto& bot = m_fields->m_bot;
            bot.setEnabled(true);

            int horizon = Mod::get()->getSettingValue<int>("decision-horizon");
            bot.update(state, horizon);

            Action newAction = bot.action();

            // Kirim input hanya saat action berubah (hindari spam handleButton)
            if (newAction != m_fields->m_lastAction) {
                // Press  → handleButton(true,  1, true)
                // Release/None → handleButton(false, 1, true)
                bool press = (newAction == Action::Press);
                this->handleButton(press, 1, true);   // button 1 = jump, player1

                m_fields->m_lastAction = newAction;
                log::debug("[TrajectoryBot] {} | confidence={:.2f}",
                    press ? "PRESS" : "RELEASE", bot.confidence());
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════
    // RESET LEVEL — HOOK BARU (sebelumnya tidak ada)
    // ═══════════════════════════════════════════════════════════════════════
    void resetLevel() {
        PlayLayer::resetLevel();

        // Bersihkan overlay saat level direset (respawn)
        if (m_fields->m_overlay)
            m_fields->m_overlay->clear();

        // Reset state bot; re-enable sesuai setting
        m_fields->m_lastAction = Action::None;
        m_fields->m_bot.setEnabled(
            Mod::get()->getSettingValue<bool>("bot-enabled")
        );
    }

    // ═══════════════════════════════════════════════════════════════════════
    // DESTROY PLAYER (kematian) — HOOK BARU (sebelumnya tidak ada)
    // ═══════════════════════════════════════════════════════════════════════
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        PlayLayer::destroyPlayer(player, obj);

        // Hapus overlay saat mati
        if (m_fields->m_overlay)
            m_fields->m_overlay->clear();

        // Disable bot — resetLevel() akan re-enable jika diperlukan
        m_fields->m_lastAction = Action::None;
        m_fields->m_bot.setEnabled(false);
    }
};

// ── Mod load log ────────────────────────────────────────────────────────────
$execute {
    log::info("[TrajectoryBot] v{} loaded | GD {} | Phase 1 active",
        Mod::get()->getVersion().toVString(),
        GEODE_GD_VERSION
    );
}
