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

## Build

Install the [Geode SDK](https://docs.geode-sdk.org/getting-started/), then run:

```sh
export GEODE_SDK=/path/to/geode
cmake -S . -B build
cmake --build build --config Release
```

The build produces a `.geode` package through `setup_geode_mod`.

For Geode 4.0.0, use bindings revision
`23c39fcda5dc660d8e92f3fb14f29b0a58a15f98`, matching CI. Clone
`https://github.com/geode-sdk/bindings`, check out that revision, and set
`GEODE_BINDINGS_REPO_PATH` to the clone's absolute path. Add
`-DSKIP_BUILDING_CODEGEN=OFF` to the CMake configure command so it builds the
matching generator. Current bindings require SDK APIs absent from Geode 4.0.0.

For Android builds, use NDK `27.3.13750724`, matching CI. Geode 4's bundled
fmt 11.0.2 fails compile-time format-string checks with NDK 29's Clang.

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
