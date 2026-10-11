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
| 1 | *(Recommended)* 100 µF electrolytic capacitor for servo power |

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

> **Power tip:** Two servos moving at once can pull more current than USB gives. That makes the Arduino reset or the servos jitter. Put a **100 µF capacitor** across servo 5V and GND (long leg / + to 5V). If problems continue, power the servos from a separate 5V supply (4×AA pack) and **connect its GND to the Arduino GND**.

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

### 2. Calibrate the sensors (type `c`)

No two photoresistors read exactly the same. If one naturally reads higher than its partner, the tracker thinks the light is off to that side even when it isn't, so it jitters or drifts. Calibration measures each sensor's quirk once and corrects for it.

1. Upload with `SENSOR_TEST_MODE = false` and open the Serial Monitor (9600 baud).
2. Lay a sheet of **plain white paper flat across the top of the `+` wall**, covering all four sensors.
3. Shine the flashlight **straight down onto the paper** from about a foot above. The paper spreads the light evenly, so all four sensors get the same amount.
4. Type **`c`** and press **Enter**. Hold everything still through the 5-second countdown and the 2-second measurement.
5. It prints something like `Calibration (100 = no correction): TL 95  TR 134  BL 105  BR 80` and saves it. The calibration **stays saved** after unplugging and re-uploading. Redo it if you swap a sensor. Type `r` to clear it.

If it says `FAILED`, one sensor read very differently from the others. Check that the paper covers all four sensors evenly and that sensor's wiring.

### 3. Direction test (type `d`)

This checks that each axis turns **toward** the light and tells you exactly what to change if it doesn't.

1. Type **`d`** and press **Enter**. The panel tilts to one side and stops.
2. Hold the flashlight **steady**, 1–2 feet away, shining **straight at the panel face** (from the direction it's now pointing).
3. **Keep the light still** while it nudges each servo back and forth (don't follow the panel).
4. It prints a result for each axis:
   ```
   DONE. Results:
     TILT (error change -57%): set  TILT_REVERSED = false   (already correct)
     PAN  (error change 43%): set  PAN_REVERSED  = true   <-- CHANGE THIS and re-upload
   ```
5. Change any setting marked **`<-- CHANGE THIS`** at the top of the code, re-upload, and run `d` again. Both should say `already correct`.
   If it says `can't tell`, aim the light more directly at the panel, move it closer, and retry.

> **Run the direction test whenever you rebuild or re-mount a servo.** A backwards setting makes that axis run away from the light into its end stop. That was the cause of the "shuffling between extremes" problem.

### 4. Set the flat angle and limits

- `TILT_FLAT_ANGLE`: the tilt servo angle where the panel faces **straight up**. It's 90 if you centered the servo horn when assembling. If your plate is flat at a different angle, change this number. The pan-flip and wrap features depend on it.
- `PAN_MIN / PAN_MAX / TILT_MIN / TILT_MAX`: narrow these if the printed parts collide or the wires get pulled near the ends of travel.

---

## Reading the Serial Monitor

While running, it prints 4 lines per second:

```
TRACK | TL:935 TR:904 BL:928 BR:948 | up/down:12% left/right:-3% | light from: TOP | pan:84 tilt:101
```

| Part | Meaning |
|------|---------|
| `TRACK` / `HOLD` | Moving toward the light / lined up and resting. Also `DARK-HOLD` (too dark), `WRAP` (big swing past an end stop), `TEST` (sensor test mode). |
| `TL:935 ...` | Raw readings, 0–1023 (same numbers as the sensor test). |
| `up/down:12%` | How much brighter the top is than the bottom (negative = bottom brighter). It starts moving above **8%** and stops below **3%**. |
| `left/right:-3%` | Same for left vs. right (negative = right brighter). |
| `pan:84 tilt:101` | Where the servos are aimed. |

Seeing `Solar tracker starting...` again in the middle of a run means the Arduino **reset**, which is almost always a servo power problem (see Troubleshooting).

---

## How the code keeps it smooth

- **Brightness conversion:** with the 10 kΩ divider, bright light squashes all readings up near 1023, so a lit sensor (990) and a shaded one (850) look almost the same. The code undoes the divider math, which spreads the readings back out about 5×, so it can actually see the shadows.
- **Calibration** cancels out differences between individual sensors.
- **Smoothing** averages readings over time to ignore electrical noise.
- **Start/stop thresholds:** it starts moving at an 8% difference but keeps going until it's under 3%, so it doesn't twitch on and off around one cutoff.
- **Small steps** (at most 1° every 40 ms), sent to the servos in fine microsecond steps instead of whole degrees, so motion is smooth and the plate has time to keep up.
- **Flat-zone buffer:** the pan direction only flips once the plate is 6° past flat, so it doesn't flip-flop when the light is overhead.
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
| `WRAP_PAN_AT_LIMITS` | `true` | Allow the big swing past a pan end stop. Set `false` to just wait at the end stop. |
| `WRAP_MIN_ERROR` / `WRAP_CONFIRM_MS` | `0.30` / `1500` | How clearly, and for how long, the light must be past the end stop before swinging. |
| `DARK_THRESHOLD` | `30` | If the average raw reading is below this, it holds still. |
| `SAMPLES_PER_READ` | `8` | Readings averaged per sensor. |
| `SERIAL_DEBUG` | `true` | Prints the status line 4× per second. |

---

## Troubleshooting

| Problem | Likely cause / fix |
|---------|--------------------|
| Runs **away** from the light / shuffles between extreme positions | A direction setting is backwards. Run the direction test (`d`). |
| Jitters back and forth when pointed at the light | Calibrate (`c`). If it still jitters, raise `START_THRESHOLD` to `0.12` or lower `GAIN` to `3`. |
| Stops a bit off from the light | Calibrate (`c`). If it's still off, lower `STOP_THRESHOLD` to `0.02`. |
| Moves away on **both** axes, or wanders randomly | Readings are inverted. Run the sensor test and check `INVERT_LDR_READINGS`. |
| Moves the wrong axis (tilts when it should turn) | Sensors are in the wrong pins, or the `+` wall isn't lined up with the tilt axis. |
| Pan direction is only wrong when tilted to one side | `TILT_FLAT_ANGLE` doesn't match the angle where your panel is actually flat. |
| `Solar tracker starting...` appears mid-run, or servos twitch randomly | The Arduino is resetting from servo power dips. Add the 100 µF capacitor, or power the servos from a separate 5V supply (4×AA) with its GND connected to the Arduino GND. |
| Never moves | Too dark (`DARK-HOLD`), already lined up (`HOLD`, so try moving the light), or `SENSOR_TEST_MODE` is still `true`. |
| Typing `c` / `d` does nothing | Make sure the Serial Monitor is at 9600 baud, then type the letter and press Enter. |
| Serial Monitor shows garbage | Set the baud rate to 9600. |
| Works with a flashlight but not in direct sunlight | In full sun all four sensors saturate near 1023. Swap the four 10 kΩ resistors for **1 kΩ** (brown-black-red) for outdoor use. |
