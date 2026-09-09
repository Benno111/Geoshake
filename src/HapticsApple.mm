#include "Haptics.hpp"

#include <Geode/Geode.hpp>

#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>

namespace geoshake {

void vibratePhone(Pulse pulse) {
    if (pulse.strength <= 0.f || pulse.durationMs == 0) return;
    auto mod = geode::Mod::get();
    if (!mod->getSettingValue<bool>("phone-vibration")) return;
    auto durationOnly = mod->getSettingValue<bool>("phone-duration-only");

    dispatch_async(dispatch_get_main_queue(), ^{
        if (durationOnly) {
            // Like Cocos Device::vibrate, iOS ignores the requested duration.
            AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
            return;
        }
        UIImpactFeedbackStyle style = pulse.strength < .34f
            ? UIImpactFeedbackStyleLight
            : pulse.strength < .67f ? UIImpactFeedbackStyleMedium : UIImpactFeedbackStyleHeavy;
        UIImpactFeedbackGenerator* generator =
            [[UIImpactFeedbackGenerator alloc] initWithStyle:style];
        [generator prepare];
        if (@available(iOS 13.0, *)) [generator impactOccurredWithIntensity:pulse.strength];
        else [generator impactOccurred];
    });
}

} // namespace geoshake
