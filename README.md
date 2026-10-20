# Input Buffer (Geode mod)

Remembers a click made a few physics ticks too early and fires it on the first
tick where it can work (touching an unused orb, or landing on the ground).

## Settings
- **Enable Buffer** - on/off.
- **Buffer Window (ticks)** - how long an early click is remembered (1-10, default 2).

## Building the .geode file

A `.geode` file is a compiled package, so it has to be built with the Geode SDK.

### Option A: locally
1. Install the Geode CLI and SDK: https://docs.geode-sdk.org/getting-started/
2. In this folder run:
   ```
   geode build
   ```
3. The result is `build/yourname.input-buffer.geode`. Drop it in your
   `Geometry Dash/geode/mods` folder (or double-click it with Geode installed).

### Option B: GitHub Actions (no local setup)
1. Push this folder to a GitHub repo.
2. Open the Actions tab and run "Build Geode Mod".
3. Download `Build Output` - it contains the combined `.geode` file.

## If the build fails
The hooks use `GJBaseGameLayer::handleButton(bool, int, bool)` and
`GJBaseGameLayer::processCommands(float)`. Geode's bindings occasionally change
signatures between GD versions; if the compiler complains, check the current
declarations in `geode-sdk/bindings` and adjust the two hook signatures in
`src/main.cpp`. Also change `id` and `developer` in `mod.json` to your own.
