#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCMotionStreak.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <unordered_map>

using namespace geode::prelude;

static std::unordered_map<CCMotionStreak*, bool> streakStates;
static bool trailExternallyTriggered = false;

static double cutFreq = 0.2;

$execute {
    listenForSettingChanges("cutting-freq", [](double value) {
        cutFreq = value;
        CCLOG("Cutting Frequency updated: %f", cutFreq);
    });
}

class $modify(CCMotionStreak) {

    struct Fields {
        float elapsedTime = 0.1f;
        bool isCutting = false;
    };

    virtual void update(float delta) {
        if (streakStates[this]) {
            m_fields->elapsedTime += delta;

            if (m_fields->elapsedTime >= cutFreq) {
                m_fields->elapsedTime -= cutFreq;

                if (m_fields->isCutting) {
                    this->stopStroke();
                } else {
                    this->resumeStroke();
                }

                m_fields->isCutting = !m_fields->isCutting;
            }
        }

        CCMotionStreak::update(delta);
    }
};

class $modify(PlayerObject) {

    void activateStreak() {
        PlayerObject::activateStreak();

        if (m_regularTrail) {
            auto streak = reinterpret_cast<CCMotionStreak*>(m_regularTrail);
            if (streak) {
                streakStates[streak] = true;
            }
        }
    }

    // just in case
    // this code is old as SHIT but i dont wanna remove it
    // im scared its gonna fuck up LOL
    void resetStreak() {
        PlayerObject::resetStreak();

        if (m_regularTrail) {
            auto streak = reinterpret_cast<CCMotionStreak*>(m_regularTrail);
            if (streak) {
                streakStates[streak] = false;
            }
        }
    }

    void hitGround(GameObject* p0, bool p1) {
        PlayerObject::hitGround(p0, p1);

        if (!m_isShip && !m_isSwing && !m_isDart && !m_isBird) {
            if (m_regularTrail) {
                auto streak = reinterpret_cast<CCMotionStreak*>(m_regularTrail);
                if (streak) {
                    streakStates[streak] = false;
                }
            }
        }
    }

    void update(float delta) {
        PlayerObject::update(delta);

        if (m_isShip || m_isSwing || m_isDart || m_isBird) {
            if (m_regularTrail) {
                auto streak = reinterpret_cast<CCMotionStreak*>(m_regularTrail);
                if (streak) {
                    streakStates[streak] = true;
                }
            }
        }

        if (trailExternallyTriggered) {
            trailExternallyTriggered = false;
        }
    }
};
