#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "Haptics.hpp"

using namespace geode::prelude;

$execute {
    ButtonSettingPressedEventV3(Mod::get(), "vibrator-debug").listen([](std::string_view button) {
        if (button != "status") return;
        auto status = geoshake::getVibratorStatus();
        log::info("Vibrator status:\n{}", status);
        FLAlertLayer::create("Vibrator status", status, "OK")->show();
    }).leak();
    ButtonSettingPressedEventV3(Mod::get(), "vibration-test").listen([](std::string_view button) {
        if (button != "test") return;
        auto mod = Mod::get();
        auto controller = mod->getSettingValue<bool>("controller-vibration");
        auto phone = mod->getSettingValue<bool>("phone-vibration");
#ifdef GEODE_IS_ANDROID
        phone = phone && (mod->getSettingValue<bool>("phone-duration-only") ||
            mod->getSettingValue<bool>("android-modern-vibration") ||
            mod->getSettingValue<bool>("android-legacy-vibration"));
#endif
        auto scale = mod->getSettingValue<double>("strength-scale");
        if ((!controller && !phone) || scale <= 0.0) {
            FLAlertLayer::create("Vibration test",
                "Enable phone or controller vibration and set haptic strength above zero. On Android, enable Cocos-style mode or a modern/legacy method. Apply changes before testing.",
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

