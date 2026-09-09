#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "Haptics.hpp"

using namespace geode::prelude;

$execute {
    ButtonSettingPressedEventV3(Mod::get(), "vibration-test").listen([](std::string_view button) {
        if (button != "test") return;
        auto mod = Mod::get();
        auto controller = mod->getSettingValue<bool>("controller-vibration");
        auto phone = mod->getSettingValue<bool>("phone-vibration");
        auto scale = mod->getSettingValue<double>("strength-scale");
        if ((!controller && !phone) || scale <= 0.0) {
            FLAlertLayer::create("Vibration test",
                "Enable phone or controller vibration and set haptic strength above zero. Apply your changes before testing.",
                "OK")->show();
            return;
        }
        auto pulse = geoshake::fromShake(0.5f, 0.5f, 0.f, static_cast<float>(scale));
        if (controller) geoshake::vibrateControllers(pulse);
        if (phone) geoshake::vibratePhone(pulse);
    }).leak();
}

class $modify(GeoShakeGameLayer, GJBaseGameLayer) {
    void shakeCamera(float duration, float strength, float interval) {
        GJBaseGameLayer::shakeCamera(duration, strength, interval);

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

