#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "Haptics.hpp"

using namespace geode::prelude;

class $modify(GeoShakeGameLayer, GJBaseGameLayer) {
    void shakeCamera(float strength, float duration, float interval) {
        GJBaseGameLayer::shakeCamera(strength, duration, interval);

        auto mod = Mod::get();
        auto pulse = geoshake::fromShake(
            strength,
            duration,
            interval,
            mod->getSettingValue<double>("strength-scale")
        );

        if (mod->getSettingValue<bool>("controller-vibration"))
            geoshake::vibrateControllers(pulse);
        if (mod->getSettingValue<bool>("phone-vibration"))
            geoshake::vibratePhone(pulse);
    }
};

