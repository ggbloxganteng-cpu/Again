#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "RuntimeState.hpp"
#include "AutoPlayer.hpp"

using namespace geode::prelude;

namespace {
    AutoPlayer g_bot;
    bool g_logged = false;
}

class $modify(TrajectoryBotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        g_bot.setEnabled(Mod::get()->getSettingValue<bool>("bot-enabled"));
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto player = this->m_player1;
        if (!player)
            return;

        RuntimeState state = RuntimeStateReader::read(player);

        if (g_bot.enabled()) {
            int horizon = Mod::get()->getSettingValue<int>("decision-horizon");
            g_bot.update(state, horizon);

            // This project intentionally stops at the decision layer.
            // The final input adapter must be bound to the exact 2.208
            // handleButton/input signature supplied by the installed SDK.
            //
            // That avoids hard-coding a wrong 2.208/2.2081 ABI call.
            if (!g_logged) {
                log::info("TrajectoryBot decision engine active; action={}, confidence={}",
                    static_cast<int>(g_bot.action()), g_bot.confidence());
                g_logged = true;
            }
        }
    }
};

$execute {
    log::info("TrajectoryBot loaded for GD {}", GEODE_GD_VERSION);
}
