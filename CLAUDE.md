# CLAUDE.md — kitelite

## What this repo is

kitelite is ESP32 Feather firmware that drives LED light shows on a kite using real-time IMU
orientation data. It reads from an ISM330DHCX (accel/gyro) + LIS3MDL (magnetometer)
FeatherWing, runs AHRS fusion, and maps orientation to LED patterns.

Built with **PlatformIO** in the Arduino/ESP32 ecosystem.

## How to work in this repo

- **Never `git add`, `git commit`, or `git push` anything**, ever, under any circumstances.
  Leave all changes as working tree modifications only.
- **Never create or switch branches** without explicit instruction from the user.
- When a task is complete, summarize what files changed and why. The user will review
  every diff, stage what they want, and commit on their own schedule.
- Before making changes that touch more than one file, propose a plan and wait for approval.
- Prefer small, focused changes over large rewrites in a single pass.
- Write code as if the user will need to read, understand, and debug it without your help.
  Clarity over cleverness, always.

## Hardware

- **MCU**: Adafruit ESP32-S3 Feather with reverse TFT display (most likely target)
- **IMU FeatherWing**: ISM330DHCX + LIS3MDL
  - ISM330DHCX: ±4000 dps gyro range, industrial-grade temperature compensation —
    correct for kite flight dynamics (high rotation rates)
  - LIS3MDL: magnetometer

## Current state

kitelite is mostly a stub. The input and display scaffolding exists but is largely empty.
Key things that are working:

- IMU sensor initialization (ISM330DHCX + LIS3MDL via I2C)
- Serial output of `Raw:`, `Uni:`, `Cal1:`, `Cal2:` lines for use with SensorCal
- Receiving calibration packets back from SensorCal (`MotionCal::CReader`)
- Calibration stored via `Adafruit_Sensor_Calibration_EEPROM`

Key things that do NOT exist yet:
- libcalib integration (kitelite still uses `Adafruit_Sensor_Calibration` directly)
- `vendor/libcalib` and `vendor/Fusion` submodules
- AHRS orientation output driving LEDs or display
- Any meaningful display code

Do not assume libcalib or Fusion are present unless you can see them in the repo.

## Repo layout

```
kitelite/
├── src/
│   ├── main.cpp
│   ├── motion.cpp        # IMU init, serial output, calibration receive (CReader)
│   └── motion.h
├── platformio.ini
├── CLAUDE.md             # this file
└── CLAUDE-coding.md      # coding style reference
```

When libcalib and Fusion are added, they will live under `vendor/` as git submodules,
and a `lib/FusionShim/` directory will provide PlatformIO a `library.json` for Fusion.

## Serial output format

`motion.cpp` (`MotionCal::TraceCalibration()`) emits four line types over serial:

**`Raw:` and `Uni:`** — emitted every update cycle at ~100Hz:
```
Raw:<ax>,<ay>,<az>,<gx>,<gy>,<gz>,<mx>,<my>,<mz>\r\n   (int16, lossy scaling)
Uni:<ax>,<ay>,<az>,<gx>,<gy>,<gz>,<mx>,<my>,<mz>\r\n   (float, SI units, full precision)
```

**`Cal1:` and `Cal2:`** — emitted periodically (every 50 / 100 cycles respectively),
echoing the current stored calibration back to SensorCal for confirmation:
```
Cal1:<accel xyz>, <gyro xyz>, <mag hard iron xyz>, <mag field>\r\n   (10 floats)
Cal2:<mag soft iron 3x3>\r\n                                         (9 floats)
```

SensorCal prefers `Uni:` over `Raw:` (full float precision). The `Cal1:`/`Cal2:` echo
is used by SensorCal to confirm the calibration was received and stored correctly.

## Calibration receive (`CReader`)

`MotionCal::CReader` is a state machine class that reads the 68-byte binary calibration
packet sent by SensorCal and applies it to `Adafruit_Sensor_Calibration`. The packet
format matches what `send_calibration()` in SensorCal's `serialdata.cpp` produces:
- 2-byte signature (0x75, 0x54)
- 16 floats: accel offsets, gyro offsets, mag hard iron, field strength, soft iron upper triangle
- 2-byte CRC16

The `IG_*` enum in `SetCalib()` documents the float layout explicitly — keep it in sync
with SensorCal's `send_calibration()` if either side changes.

## Coding style

See `CLAUDE-coding.md` in the repo root for full reference. Key rules:

- Tabs (not spaces), 4-wide
- `m_` prefix on all class/struct members
- `g_` prefix on file-static globals; `s_` on function-static variables
- No `enum class` — plain enums, integer-convertible
- `ASSERT` / `CASSERT` for correctness checks (embedded-safe variants)
- `nullptr` not `NULL`
- C++ casts only — never C-style casts
- `//` comments only
- `BB(username)` for known improvement areas; `NOTE(username)` for non-obvious decisions

## Vendor submodules

None yet. When added, all vendored dependencies will live under `vendor/`.
