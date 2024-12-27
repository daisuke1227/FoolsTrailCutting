#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCMotionStreak.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <unordered_map>

using namespace geode::prelude;

static std::unordered_map<CCMotionStreak*, bool> streakStates;
static bool trailExternallyTriggered = false;


static double cutFreq = Mod::get()->getSettingValue<double>("cutting-freq");

$execute {
    listenForSettingChanges("cutting-freq", [](double value) {
        cutFreq = value;
    });
}

class $modify(CCMotionStreak) {
    struct Fields {
        float elapsedTime = 0.0f;
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
            auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
            if (streak) {
                streakStates[streak] = true;
            }
        }
    }

    void resetStreak() {
        PlayerObject::resetStreak();

        if (m_regularTrail) {
            auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
            if (streak) {
                streakStates[streak] = false;
            }
        }
    }

    void bumpPlayer(float p0, int p1, bool p2, GameObject* p3) {
        PlayerObject::bumpPlayer(p0, p1, p2, p3);

        if (m_regularTrail) {
            auto streak = static_cast<CCMotionStreak*>(m_regularTrail); 
            if (streak) {
                streakStates[streak] = true;
                trailExternallyTriggered = true;
            }
        }
    }

    void hitGround(GameObject* p0, bool p1) {
        PlayerObject::hitGround(p0, p1);

        if (!m_isShip && !m_isSwing && !m_isDart && !m_isBird) {
            if (m_regularTrail) {
                auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
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
                auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
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
