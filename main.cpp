// Input Buffer - Geode mod for Geometry Dash 2.2074
//
// Idea: if you click a few ticks too early (nothing to jump off yet), the mod
// remembers the click. On the first tick where the click could actually do
// something (touching an unused orb, or landing on the ground), it re-sends
// the click to the game. If nothing becomes available within the buffer window,
// the remembered click is thrown away.
//
// NOTE: this is a client-side assist. Don't use it on anything where legit
// completions matter (Demonlist, leaderboard submissions, etc.).

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {

    struct Slot {
        bool pending = false;      // an early click is waiting for its moment
        int ticksLeft = 0;         // how long it stays valid
        bool realDown = false;     // what the player's finger is really doing
        bool releaseNext = false;  // we injected a tap and still have to let go
    };

    // index 0 = player 1, index 1 = player 2
    Slot g_slots[2];

    void resetAll() {
        g_slots[0] = Slot{};
        g_slots[1] = Slot{};
    }

    bool modEnabled() {
        return Mod::get()->getSettingValue<bool>("enabled");
    }

    int windowTicks() {
        return static_cast<int>(Mod::get()->getSettingValue<int64_t>("buffer-ticks"));
    }

    // Would a click right now actually do something?
    bool canActNow(PlayerObject* p, bool isPlayer1) {
        if (!p) return false;

        // touching an orb that hasn't been used yet
        for (auto* ring : p->m_touchedRings) {
            if (!ring) continue;
            bool used = isPlayer1 ? ring->m_hasBeenActivatedP1
                                  : ring->m_hasBeenActivatedP2;
            if (!used) return true;
        }

        // standing on the ground in a mode where a tap makes you jump
        bool holdMode = p->m_isShip || p->m_isDart || p->m_isSwing;
        if (!holdMode && p->m_isOnGround) return true;

        return false;
    }

}

class $modify(BufferBaseLayer, GJBaseGameLayer) {
    // Every real press/release passes through here.
    void handleButton(bool down, int button, bool isPlayer1) {
        if (modEnabled() && button == 1) { // 1 = jump (2/3 are platformer left/right)
            auto& s = g_slots[isPlayer1 ? 0 : 1];
            s.realDown = down;

            if (down) {
                auto* p = isPlayer1 ? m_player1 : m_player2;
                // If the click already works, vanilla handles it. Only buffer
                // clicks that would otherwise be wasted.
                if (!canActNow(p, isPlayer1)) {
                    s.pending = true;
                    s.ticksLeft = windowTicks();
                }
            }
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    // Runs once per physics step. We act BEFORE the original so the injected
    // click is seen by this step's collision pass.
    void processCommands(float dt) {
        if (modEnabled()) {
            for (int i = 0; i < 2; ++i) {
                auto& s = g_slots[i];
                bool isP1 = (i == 0);
                auto* p = isP1 ? m_player1 : m_player2;

                // finish a tap we injected on an earlier step
                if (s.releaseNext) {
                    s.releaseNext = false;
                    if (!s.realDown) {
                        GJBaseGameLayer::handleButton(false, 1, isP1);
                    }
                }

                if (s.pending) {
                    if (canActNow(p, isP1)) {
                        s.pending = false;
                        GJBaseGameLayer::handleButton(true, 1, isP1);
                        if (!s.realDown) s.releaseNext = true;
                    } else if (--s.ticksLeft <= 0) {
                        s.pending = false; // window expired
                    }
                }
            }
        }
        GJBaseGameLayer::processCommands(dt);
    }
};

class $modify(BufferPlayLayer, PlayLayer) {
    void resetLevel() {
        resetAll(); // don't carry a buffered click across a respawn
        PlayLayer::resetLevel();
    }
};
