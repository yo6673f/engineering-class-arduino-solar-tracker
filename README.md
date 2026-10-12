# Dual-Axis Arduino Solar Tracker

**ENGR 102B, University of Arizona**: a solar tracker built with the Arduino Uno R3 kit. Four photoresistors sit in the quadrants of a `+` shaped wall above the solar panel. Two servos aim the panel **toward** the light: one tilts it up and down, the other turns it clockwise and counterclockwise.

---

## Repository layout

```
├── WIRING.md                          <- hole-by-hole breadboard guide
├── platformio.ini                     <- PlatformIO project (VS Code)
├── src/
│   └── main.cpp                       <- PlatformIO code
└── arduino/
    └── SolarTracker/
        └── SolarTracker.ino           <- Arduino IDE code (identical to main.cpp)
```

`src/main.cpp` and `arduino/SolarTracker/SolarTracker.ino` contain **exactly the same code**. If you change one, copy the whole file over the other so the team stays in sync.

---

## Parts (all from the Uno R3 kit)

| Qty | Part |
|----|------|
| 1 | Arduino Uno R3 + USB cable |
| 4 | Photoresistors (LDRs) |
| 4 | 10 kΩ resistors (brown-black-orange) |
| 2 | Micro servos (SG90 / 9g) |
| 1 | Breadboard + jumper wires |

---

## Wiring

> **Step-by-step, hole-by-hole breadboard instructions are in [`WIRING.md`](WIRING.md).** The summary is below.

### Photoresistors

Each LDR is half of a **voltage divider**. Wire all four the same way:

```
   5V ──── LDR ────┬──── Analog pin (A0–A3)
                   │
                 10 kΩ
                   │
  GND ─────────────┘
```

With this wiring, **more light gives a bigger reading** (0–1023), which is what the code expects. If you wire one with the LDR and resistor swapped, that sensor reads backwards. See the sensor test below.

| Sensor position | Arduino pin | Wire color |
|-----------------|-------------|------------|
| Top-Left        | **A0**      | Yellow     |
| Top-Right       | **A1**      | Red        |
| Bottom-Left     | **A2**      | Blue       |
| Bottom-Right    | **A3**      | Green      |

### Servos

| Servo wire | Connects to |
|------------|-------------|
| Brown / Black | GND |
| Red | 5V |
| Orange / Yellow (signal), **pan** servo (turns CW/CCW) | **Pin 9** |
| Orange / Yellow (signal), **tilt** servo (tilts up/down) | **Pin 10** |

> **Power tip:** Two servos moving at once can pull more current than USB gives. That makes the Arduino reset or the servos jitter. If that happens, power the servos from a separate 5V supply (4×AA pack) and **connect its GND to the Arduino GND**. (We don't use a capacitor: one with a leg in a servo's signal row stops that servo from moving.)

---

## Sensor placement (important!)

Looking straight down at the face of the panel:

```
                  TOP
           +------+------+
           |  TL  |  TR  |
    LEFT   +------+------+   RIGHT
           |  BL  |  BR  |
           +------+------+
                 BOTTOM
```

- One arm of the `+` wall must run **parallel to the tilt axis** (the axis the tilt servo rotates around). Then **top vs. bottom** lines up with tilting and **left vs. right** lines up with turning. If the `+` is turned 45°, the axes get mixed up.
- "Top" can be either side of the tilt axis. The code doesn't care which side you call top, as long as the wiring matches the table above. The flashlight test below confirms the directions.

### How it finds the light

When light comes from an angle, the `+` wall shades the sensors on the far side. The code compares:

- **Top pair vs. bottom pair** → moves the **tilt** servo toward the brighter side
- **Left pair vs. right pair** → moves the **pan** servo toward the brighter side

It keeps stepping until all four sensors read about the same. At that point the panel faces the light. It compares *ratios*, not raw numbers, so it works the same under a desk lamp or in full Arizona sun.

Two details are handled for you:

1. **Tilting past flat:** when the plate tilts over to the other side, the panel is effectively upside down relative to the pan servo. The code flips the pan direction automatically, so it still turns *toward* the light.
2. **Pan end stops:** the pan servo only turns 180°. If the light is farther around than that, the tracker swings pan to the other end and mirrors the tilt. That pose points the panel the same way, and tracking continues from there. It only does this when the light is *clearly* past the end stop for over a second, and it can't swing straight back. You'll see a slow, big swing when it happens. That's normal.

---

## Option A: PlatformIO (VS Code)

1. Install **VS Code** and the **PlatformIO IDE** extension.
2. In PlatformIO: **Open Project** → choose this repository's folder (the one with `platformio.ini`).
3. Plug in the Uno.
4. Click **✓ Build**, then **→ Upload** in the bottom status bar.
5. Click the **🔌 Serial Monitor** button (it's already set to 9600 baud).

PlatformIO downloads the Servo library automatically (listed in `platformio.ini`).

## Option B: Arduino IDE

1. Open `arduino/SolarTracker/SolarTracker.ino` in the Arduino IDE.
   (Keep the file inside its `SolarTracker` folder; the Arduino IDE requires the folder and file names to match.)
2. **Tools → Board → Arduino Uno**, and **Tools → Port →** your Uno's port.
3. Click **Upload**.
4. **Tools → Serial Monitor** and set it to **9600 baud**.

The Servo library comes with the Arduino IDE, so there's nothing to install.

---

## First-run checklist (do these in order)

### 1. Sensor test: are the wires in the right pins?

1. In the code, set `SENSOR_TEST_MODE = true` and upload. The servos go to the middle and **stay still**.
2. Open the Serial Monitor. You'll see lines like:
   ```
   TEST | TL:612 TR:598 BL:605 BR:590 | up/down:2% left/right:1% | light from: CENTER | pan:90 tilt:90
   ```
3. Cover **one sensor at a time** with your finger:
   - Its number should go **down** (darker), and only that one should change much.
   - If the wrong label changes, swap the wires on the breadboard (or the pin numbers at the top of the code).
4. Shine a flashlight on one sensor. Its number should go **up**.
   - If **all** numbers go down with more light, set `INVERT_LDR_READINGS = true`.
   - If only **one** goes backwards, that divider is wired backwards. Swap its LDR and 10 kΩ resistor.
5. Shine the light from the top side of the `+`. The monitor should say `light from: TOP`. Repeat for each side.
6. Set `SENSOR_TEST_MODE = false` when done.

### 2. Center the servo arms (`SERVO_CENTER_MODE`)

If the tracker leans toward one corner even when it's "in the middle", the servo arms were attached at the wrong angle, so the servo's 90° isn't the middle of your mechanism. Fix it once, mechanically:

1. In the code, set `SERVO_CENTER_MODE = true` and upload. Both servos go to **exactly 90°** and stay there (no scan, no tracking).
2. **Pan servo:** unscrew the small screw in the center of the servo arm (horn), pull the arm straight off the servo shaft **without turning the shaft**, and push it back on so the tracker points **straight ahead, in the middle of its swing**. Put the screw back.
3. **Tilt servo:** same thing, re-seated so the plate is **flat** (panel facing straight up).
4. The splines on the shaft only allow certain angles, so you may be a few degrees off. That's fine: fix the last bit with `TILT_FLAT_ANGLE` in step 3.
5. Set `SERVO_CENTER_MODE = false` and re-upload.

> Unplug USB before forcing anything by hand. Never twist a servo's shaft with your fingers while it's powered; it strips the gears.

### 3. Find your servo limits (type `h`, `p`, `t`)

**Do this right after centering the arms.** If the code tells a servo to go further than your 3D printed parts allow, the servo stalls against the part. It buzzes, jitters, heats up, causes power dips, and the tracker thinks it's pointing somewhere it isn't. That was behind the "stuck in one corner" behavior.

1. Upload and open the Serial Monitor (9600 baud). Let the startup scan finish.
2. Type **`h`** + Enter. Both servos go to the middle (90°) and tracking pauses.
3. **Tilt limits:** type `t70` + Enter, then `t60`, `t50`, ... watching the plate. When it **stops moving, hits something, or the servo buzzes**, note that angle, add about **5°** back, and that's `TILT_MIN`. Then go up (`t110`, `t120`, ...) to find `TILT_MAX`.
4. **Pan limits:** type `h` + Enter, then do the same with `p` (`p70`, `p50`, ... and `p110`, `p130`, ...) for `PAN_MIN` and `PAN_MAX`.
5. **Flat angle:** type `t90` + Enter. If the panel isn't facing straight up, try nearby numbers (`t85`, `t95`, ...) until it is. That number is `TILT_FLAT_ANGLE`.
6. Put the numbers in the code:
   ```cpp
   const int PAN_MIN  = 10;     // <- your numbers
   const int PAN_MAX  = 170;
   const int TILT_MIN  = 30;
   const int TILT_MAX  = 150;
   const int TILT_FLAT_ANGLE = 90;
   ```
   Re-upload. The defaults (pan 10–170, tilt 30–150) are already safer than 0–180, but your build may need narrower.

### 4. Calibrate the sensors (type `c`)

No two photoresistors read exactly the same. Calibration measures each one's quirk once and corrects for it.

1. Lay a sheet of **plain white paper flat across the top of the `+` wall**, covering all four sensors.
2. Shine the flashlight **straight down onto the paper** from about a foot above.
3. Type **`c`** + Enter and hold still through the countdown and measurement.
4. It prints something like `Calibration (100 = no correction): TL 95  TR 134  BL 105  BR 80` and saves it. It **stays saved** after unplugging and re-uploading. Redo it if you swap a sensor. Type `r` to clear it.
5. Remove the paper and type **`g`** + Enter to start tracking.

If it says `FAILED`:
- **"maxed out"**: the light is too bright for the sensors. Hold the flashlight farther from the paper (or use the room light).
- **"reads almost 0"**: that sensor is covered, unplugged, or has a loose wire.

A **"Note: ... needed a big correction"** message still saves. It just means that sensor reads quite differently from the others. That's fine, but if the light on the paper was uneven, redo it, and if that sensor's leg was repaired, check the joint.

### 5. Normal use

Every time it powers on, the tracker:
1. **Moves to the middle** (90/90).
2. **Scans** for the brightest direction (about 20 seconds). **Hold your flashlight still during the scan.**
3. **Checks the servo directions** using what it saw during the scan, and fixes any that are backwards:
   ```
   CHECK: which way is toward the light?
     TILT: normal  (sure: 99%)
     PAN : reversed  (sure: 92%)  <- auto-corrected
   ```
   If you see `auto-corrected` every time, you can set `TILT_REVERSED` / `PAN_REVERSED` in the code to match so it starts out right, but you don't have to.
4. **Tracks.** If it loses the light (readings drop below half of what they were right after the scan, for 5 seconds), it prints `LOST the light` and scans again.

Type **`g`** + Enter at any time to make it rescan and restart tracking.

### Serial Monitor commands

| Type (then Enter) | What it does |
|-------------------|--------------|
| `h` | Go **home** (both servos to the middle) and pause tracking |
| `p90` | Move **pan** to 90° and pause (any number 0–180) |
| `t45` | Move **tilt** to 45° and pause |
| `g` | **Go:** scan for the light, check directions, start tracking |
| `c` | **Calibrate** the sensors (paper + flashlight) |
| `r` | **Reset** calibration |

---

## Reading the Serial Monitor

While running, it prints 4 lines per second:

```
TRACK | TL:935 TR:904 BL:928 BR:948 | up/down:12% left/right:-3% | light from: TOP | pan:84 tilt:101
```

| Part | Meaning |
|------|---------|
| `TRACK` / `HOLD` | Moving toward the light / lined up and resting. Also `MANUAL` (paused after `h`/`p`/`t`, type `g` to resume), `DARK-HOLD` (too dark), `WRAP` (big swing past an end stop), `TEST` (sensor test mode). |
| `TL:935 ...` | Raw readings, 0–1023 (same numbers as the sensor test). |
| `up/down:12%` | How much brighter the top is than the bottom (negative = bottom brighter). It starts moving above **8%** and stops below **3%**. |
| `left/right:-3%` | Same for left vs. right (negative = right brighter). |
| `panel:5.73V (max 6.10)` | Solar panel voltage now, and the highest seen since startup. With `PANEL_LOAD_OHMS` set, also watts. |
| `pan:84 tilt:101` | Where the servos are aimed. `(at limit)` means one is at a limit from the code. |

Seeing `Solar tracker starting...` again in the middle of a run means the Arduino **reset**, which is almost always a servo power problem (see Troubleshooting).

---

## How the code keeps it smooth

- **Brightness conversion:** with the 10 kΩ divider, bright light squashes all readings up near 1023, so a lit sensor (990) and a shaded one (850) look almost the same. The code undoes the divider math, which spreads the readings back out about 5×, so it can actually see the shadows.
- **Calibration** cancels out differences between individual sensors.
- **Smoothing** averages readings over time to ignore electrical noise.
- **Start/stop thresholds:** it starts moving at an 8% difference but keeps going until it's under 3%, so it doesn't twitch on and off around one cutoff.
- **Small steps** (at most 1° every 40 ms), sent to the servos in fine microsecond steps instead of whole degrees, so motion is smooth and the plate has time to keep up.
- **Flat-zone buffer:** the pan direction only flips once the plate is 6° past flat, so it doesn't flip-flop when the light is overhead.
- **Startup scan + direction check:** it starts out pointed at the light, and checks for itself which way each servo has to turn, so a servo mounted backwards doesn't send it running away from the light.
- **Lost-light recovery:** if it ends up somewhere much darker than what the scan found, it scans again instead of sitting in a corner.
- **Safe wrap-around:** it only swings to the other end stop if the light has been *clearly* past the end (30% difference) for 1.5 seconds. Once it has, it can't swing back until the pan has moved 20° on its own, so it can never bounce between end stops. The swing itself is slowed down to avoid power dips.

---

## Tuning

All settings are constants near the top of the code:

| Setting | Default | What it does |
|---------|---------|--------------|
| `START_THRESHOLD` | `0.08` | Starts moving when one side is 8% brighter. Raise it (e.g. `0.12`) if it still twitches. |
| `STOP_THRESHOLD` | `0.03` | Keeps moving until the difference is under 3%. Lower = more precise aiming. Must be smaller than `START_THRESHOLD`. |
| `GAIN` | `4.0` | How hard it pushes toward the light. Higher = faster but may overshoot and wobble. |
| `MAX_STEP` | `1.0` | Most degrees it moves per update. Lower = smoother and slower. |
| `SMOOTHING` | `0.3` | 0–1. Lower = smoother readings but slower reactions. |
| `LOOP_INTERVAL_MS` | `40` | Time between updates. |
| `FLAT_HYSTERESIS` | `6.0` | Degrees past flat before the pan direction flips. |
| `PAN_MIN/MAX`, `TILT_MIN/MAX` | `10/170`, `30/150` | Servo limits. **Set these for your build** (see step 3). |
| `AUTO_DIRECTION_CHECK` | `true` | Figure out the servo directions during each scan. |
| `SCAN_ON_STARTUP` | `true` | Scan for the light every time it powers on. |
| `LOST_FRACTION` / `LOST_TIME_MS` | `0.50` / `5000` | Rescan if readings stay below half of what they were after the scan for 5 seconds. |
| `WRAP_PAN_AT_LIMITS` | `true` | Allow the big swing past a pan end stop. Set `false` to just wait at the end stop. |
| `WRAP_MIN_ERROR` / `WRAP_CONFIRM_MS` | `0.30` / `1500` | How clearly, and for how long, the light must be past the end stop before swinging. |
| `DARK_THRESHOLD` | `30` | If the average raw reading is below this, it holds still. |
| `SAMPLES_PER_READ` | `8` | Readings averaged per sensor. |
| `PANEL_MONITOR` | `true` | Show the solar panel voltage (A4) in the status line. |
| `ARDUINO_VOLTS` | `5.0` | The Arduino's real 5V (measure it with a multimeter for better accuracy). |
| `PANEL_LOAD_OHMS` | `0` | Load resistor across the panel. Set it (e.g. `220`) to also show watts. |
| `SERIAL_DEBUG` | `true` | Prints the status line 4× per second. |

---

## Troubleshooting

| Problem | Likely cause / fix |
|---------|--------------------|
| Servo buzzes, jitters, or the tracker gets "stuck" in a corner | A servo is pushing against a hard stop. Find your real limits with `h`, `p`, `t` (step 3) and set `PAN_MIN/MAX`, `TILT_MIN/MAX`. |
| The `pan:` / `tilt:` numbers change but the readings don't | The servo isn't actually moving: it's at a hard stop, the horn is slipping on the servo, or the servo isn't getting power. Use `p`/`t` to check that each servo really moves. |
| All four readings dip at the same moment, sometimes `DARK-HOLD` flickers | Power dips from a straining servo. Fix the limits first; if it continues, use a separate 5V supply for the servos (GND connected to the Arduino GND). |
| Runs away from the light | Make sure the light was steady during the startup scan, then type `g` to rescan. The `CHECK:` lines should say it's sure. |
| `CHECK:` says `can't tell` | The light was too weak or too even during the scan. Use a brighter flashlight, hold it closer and still, and type `g`. |
| Jitters back and forth when pointed at the light | Calibrate (`c`). If it still jitters, raise `START_THRESHOLD` to `0.12` or lower `GAIN` to `3`. |
| Stops a bit off from the light | Calibrate (`c`). If it's still off, lower `STOP_THRESHOLD` to `0.02`. |
| Moves the wrong axis (tilts when it should turn) | Sensors are in the wrong pins, or the `+` wall isn't lined up with the tilt axis. |
| It keeps scanning over and over | Look for `Solar tracker starting...` between scans. If it's there, the Arduino is **resetting** from servo power dips (see next row). If not, look at the `LOST the light (average reading ..., was ...)` line: the light dropped to less than half. Keep the flashlight steady and pointed at the tracker. |
| `Solar tracker starting...` appears mid-run | The Arduino is resetting from servo power dips. Two servos can be too much for USB. Fix the limits, and power the servos from a separate 5V supply (4×AA): its `+` to the servo-side `+` rail, its `−` to the servo-side `−` rail, unplug the Arduino's 5V wire from that rail, keep the GND wire. |
| Sensor readings are usually above ~950 | The sensors are close to maxed out, which makes tracking and calibration less precise. Swap the four 10 kΩ resistors for **2 kΩ** (red-black-red) or **1 kΩ** (brown-black-red), same holes. |
| Never moves | Too dark (`DARK-HOLD`), lined up already (`HOLD`), paused (`MANUAL`, so type `g`), or `SENSOR_TEST_MODE` is still `true`. |
| Typing commands does nothing | Serial Monitor must be at 9600 baud. Type the command, then press Enter. |
| Works with a flashlight but not in direct sunlight | In full sun all four sensors saturate near 1023. Swap the four 10 kΩ resistors for **1 kΩ** (brown-black-red) for outdoor use. |
