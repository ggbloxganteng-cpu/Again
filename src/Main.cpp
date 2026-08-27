#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "RuntimeState.hpp"
#include "AutoPlayer.hpp"
#include "Predictor.hpp"
#include "Overlay.hpp"

using namespace geode::prelude;

class $modify(TrajectoryBotPlayLayer, PlayLayer) {

    struct Fields {
        Ref<Overlay> m_overlay   = nullptr;
        AutoPlayer   m_bot;
        Action       m_lastAction = Action::None;
    };

    // ═══════════════════════════════════════════════════════════════
    // INIT
    // ═══════════════════════════════════════════════════════════════
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto* ov = Overlay::create();
        if (ov) {
            ov->setPosition(CCPointZero);
            ov->setAnchorPoint(CCPointZero);
            ov->setZOrder(999);

            // m_gameLayer tidak ada di binding GD 2.2081 → pakai PlayLayer langsung
            this->addChild(ov);
            m_fields->m_overlay = ov;

            log::debug("[TrajectoryBot] Overlay attached to PlayLayer");
        } else {
            log::warn("[TrajectoryBot] Overlay::create() gagal");
        }

        m_fields->m_bot.setEnabled(
            Mod::get()->getSettingValue<bool>("bot-enabled")
        );

        return true;
    }

    // ═══════════════════════════════════════════════════════════════
    // UPDATE
    // ═══════════════════════════════════════════════════════════════
    void update(float dt) {
        PlayLayer::update(dt);

        // Ref<Overlay> tidak pakai auto* — langsung auto (operator-> tetap jalan)
        auto ov = m_fields->m_overlay;
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

        // ── TRAJECTORY DRAW ─────────────────────────────────────────
        if (trajOn) {
            int frames = Mod::get()->getSettingValue<int>("prediction-frames");

            Action vizAction = (botOn && m_fields->m_lastAction != Action::None)
                ? m_fields->m_lastAction : Action::None;

            auto cand = predictor.evaluate(state, vizAction, frames);

            TrajectoryStatus tStatus;
            if      (cand.unsafe)                      tStatus = TrajectoryStatus::Danger;
            else if (vizAction == Action::Press)       tStatus = TrajectoryStatus::Press;
            else if (vizAction == Action::Release)     tStatus = TrajectoryStatus::Release;
            else                                       tStatus = TrajectoryStatus::Safe;

            ov->drawTrajectory(cand.path, tStatus);
        } else {
            ov->clear();
        }

        // ── AUTO PLAYER ─────────────────────────────────────────────
        if (botOn) {
            auto& bot = m_fields->m_bot;
            bot.setEnabled(true);

            int horizon = Mod::get()->getSettingValue<int>("decision-horizon");
            bot.update(state, horizon);

            Action newAction = bot.action();
            if (newAction != m_fields->m_lastAction) {
                bool press = (newAction == Action::Press);
                this->handleButton(press, 1, true);
                m_fields->m_lastAction = newAction;
                log::debug("[TrajectoryBot] {} | confidence={:.2f}",
                    press ? "PRESS" : "RELEASE", bot.confidence());
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // RESET LEVEL
    // ═══════════════════════════════════════════════════════════════
    void resetLevel() {
        PlayLayer::resetLevel();

        auto ov = m_fields->m_overlay;
        if (ov) ov->clear();

        m_fields->m_lastAction = Action::None;
        m_fields->m_bot.setEnabled(
            Mod::get()->getSettingValue<bool>("bot-enabled")
        );
    }

    // ═══════════════════════════════════════════════════════════════
    // DESTROY PLAYER (mati)
    // ═══════════════════════════════════════════════════════════════
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        PlayLayer::destroyPlayer(player, obj);

        auto ov = m_fields->m_overlay;
        if (ov) ov->clear();

        m_fields->m_lastAction = Action::None;
        m_fields->m_bot.setEnabled(false);
    }
};

$execute {
    log::info("[TrajectoryBot] v{} loaded | GD {} | Phase 1",
        Mod::get()->getVersion().toVString(),
        GEODE_GD_VERSION
    );
}
