#include "Haptics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#if __has_include(<SDL2/SDL.h>)
# include <SDL2/SDL.h>
# define GEOSHAKE_HAS_SDL 1
#elif __has_include(<SDL.h>)
# include <SDL.h>
# define GEOSHAKE_HAS_SDL 1
#else
# define GEOSHAKE_HAS_SDL 0
#endif

namespace geoshake {

Pulse fromShake(float strength, float duration, float interval, float scale) {
    auto safeStrength = std::isfinite(strength) ? std::abs(strength) : 0.f;
    auto safeScale = std::isfinite(scale) ? std::max(0.f, scale) : 1.f;
    auto seconds = std::max(
        std::isfinite(duration) ? std::max(0.f, duration) : 0.f,
        std::isfinite(interval) ? std::max(0.f, interval) : 0.f
    );

    return {
        std::clamp(safeStrength * safeScale, 0.f, 1.f),
        static_cast<unsigned>(std::clamp(seconds * 1000.f, 0.f, 60000.f))
    };
}

void vibrateControllers(Pulse pulse) {
#if GEOSHAKE_HAS_SDL
    if (pulse.strength <= 0.f || pulse.durationMs == 0) return;

    auto magnitude = static_cast<std::uint16_t>(pulse.strength * 65535.f);
    auto count = SDL_NumJoysticks();
    for (int index = 0; index < count; ++index) {
        if (!SDL_IsGameController(index)) continue;
        if (auto controller = SDL_GameControllerOpen(index)) {
            SDL_GameControllerRumble(controller, magnitude, magnitude, pulse.durationMs);
            SDL_GameControllerClose(controller);
        }
    }
#else
    (void)pulse;
#endif
}

#if !defined(GEODE_IS_ANDROID) && !defined(GEODE_IS_IOS)
void vibratePhone(Pulse) {}
#endif

} // namespace geoshake

