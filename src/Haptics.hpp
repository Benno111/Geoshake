#pragma once

namespace geoshake {

struct Pulse {
    float strength;
    unsigned durationMs;
};

// Converts Geometry Dash's Shake Trigger arguments without inventing a second
// haptic configuration. The interval is used as a lower bound for short shakes.
Pulse fromShake(float strength, float duration, float interval, float scale);

void vibrateControllers(Pulse pulse);
void vibratePhone(Pulse pulse);

} // namespace geoshake

