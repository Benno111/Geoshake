# GeoShake

GeoShake is a Geode mod that mirrors Geometry Dash's **Shake Trigger** to haptic
hardware. It uses the trigger's own strength, duration, and interval, so level
creators do not need to place or configure a second trigger.

## Platforms

| Platform | Controller | Phone |
| --- | --- | --- |
| Windows | SDL-compatible controllers | — |
| macOS | SDL-compatible controllers | — |
| Android | SDL-compatible controllers | Android vibrator |
| iOS | SDL-compatible controllers | UIKit haptics |

Controller and phone vibration can be disabled independently. A global strength
scale is provided for accessibility and hardware differences.

Use **Test vibration** in the mod settings to send a half-second pulse to enabled
devices (a single impact on iOS). The test uses saved settings, so apply changes
to the vibration toggles or haptic strength before testing.

## Build

This version targets Geometry Dash **2.2081** (the 2.208 patch release) on all
listed platforms and requires **Geode 5.10.1** or a compatible newer 5.x loader.
Use Geode SDK **5.10.1**, CMake **3.25+**, and a compiler with **C++23** support
(Clang **19+** or MSVC **19.44+**).

Install the [Geode SDK](https://docs.geode-sdk.org/getting-started/), check out
bindings revision `7f6c2a75742856de88dad354e576dcff8a28e881` from
`https://github.com/geode-sdk/bindings`, then run:

```sh
export GEODE_SDK=/path/to/geode
export GEODE_BINDINGS_REPO_PATH=/path/to/bindings
cmake -S . -B build -DSKIP_BUILDING_CODEGEN=OFF
cmake --build build --config Release
```

The build produces a `.geode` package through `setup_geode_mod`.

The pinned bindings and locally built generator match CI. Use a fresh build
directory when upgrading from the Geode 4 version.

For Android builds, use NDK `29.0.14206865`, matching CI. NDK 27's Clang 18 is
too old for Geode 5.10.1. The current workflow
builds Android64; other platforms require their respective toolchains.

## Behavior and limitations

- The original camera shake always runs before haptics, even when vibration is off.
- Strength is clamped to the range supported by haptic hardware.
- Durations are capped at 60 seconds to protect against malformed level data.
- iOS's public UIKit impact API exposes intensity but not an arbitrary duration;
  each Shake Trigger therefore produces one impact with matching intensity.
- A platform build without SDL controller-rumble headers remains functional and
  simply disables controller vibration.

## License

MIT; see [LICENSE](LICENSE).
