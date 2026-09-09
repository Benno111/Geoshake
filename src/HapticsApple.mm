#include "Haptics.hpp"

#import <UIKit/UIKit.h>

namespace geoshake {

void vibratePhone(Pulse pulse) {
    if (pulse.strength <= 0.f || pulse.durationMs == 0) return;

    dispatch_async(dispatch_get_main_queue(), ^{
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
