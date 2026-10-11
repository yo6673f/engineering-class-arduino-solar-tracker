# Dual-Axis Arduino Solar Tracker

**ENGR 102B, University of Arizona**: a solar tracker built with the Arduino Uno R3 kit. Four photoresistors sit in the quadrants of a `+` shaped wall above the solar panel. Two servos aim the panel **toward** the light: one tilts it up and down, the other turns it clockwise and counterclockwise.

---

## Repository layout

```
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

| Sensor position | Arduino pin | Wire color (fill in) |
|-----------------|-------------|----------------------|
| Top-Left        | **A0**      |                      |
| Top-Right       | **A1**      |                      |
| Bottom-Left     | **A2**      |                      |
| Bottom-Right    | **A3**      |                      |

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
2. **Pan end stops:** the pan servo only turns 180°. If the light is farther around than that, the tracker swings pan to the other end and mirrors the tilt. That pose points the panel the same way, and tracking continues from there. You'll see a big swing when this happens. That's normal.

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
   TEST | TL:612 TR:598 BL:605 BR:590 | light from: CENTER | pan:90 tilt:90
   ```
3. Cover **one sensor at a time** with your finger:
   - Its number should go **down** (darker), and only that one should change much.
   - If the wrong label changes, swap the wires on the breadboard (or the pin numbers at the top of the code).
4. Shine a flashlight on one sensor. Its number should go **up**.
   - If **all** numbers go down with more light, set `INVERT_LDR_READINGS = true`.
   - If only **one** goes backwards, that divider is wired backwards. Swap its LDR and 10 kΩ resistor.
5. Shine the light from the top side of the `+`. The monitor should say `light from: TOP`. Repeat for each side.
6. Set `SENSOR_TEST_MODE = false` when done.

### 2. Direction test: does it move *toward* the light?

1. Upload with `SENSOR_TEST_MODE = false`.
2. **Tilt:** shine the flashlight from the top side. The panel should tip toward the light. Try the bottom side too.
3. **Pan:** first let the plate tilt clearly to one side (not flat). Then shine the light from the left, then the right. The panel should turn toward it.
4. If an axis moves **away** from the light, flip its setting at the top of the code and re-upload:
   ```cpp
   const bool TILT_REVERSED = false;   // change to true if tilt runs away from the light
   const bool PAN_REVERSED  = false;   // change to true if pan runs away from the light
   ```
   You only need to fix pan once. The code handles the other side of flat automatically.

### 3. Set the flat angle and limits

- `TILT_FLAT_ANGLE`: the tilt servo angle where the panel faces **straight up**. It's 90 if you centered the servo horn when assembling. If your plate is flat at a different angle, change this number. The pan-flip and wrap features depend on it.
- `PAN_MIN / PAN_MAX / TILT_MIN / TILT_MAX`: narrow these if the printed parts collide or the wires get pulled near the ends of travel.

---

## Tuning

All settings are constants near the top of the code:

| Setting | Default | What it does |
|---------|---------|--------------|
| `DEADBAND` | `0.05` | Differences under 5% are ignored. Raise it if the panel jitters back and forth. Lower it for more precise aiming. |
| `GAIN` | `20.0` | How hard it pushes toward the light. Higher = faster but may overshoot. |
| `MAX_STEP` | `2.0` | Most degrees it moves per loop. Lower = smoother/slower. |
| `LOOP_DELAY_MS` | `25` | Pause between updates. |
| `DARK_THRESHOLD` | `30` | If the average reading is below this, it holds still (too dark to track). |
| `SAMPLES_PER_READ` | `8` | Readings averaged per sensor to reduce noise. |
| `SERIAL_DEBUG` | `true` | Prints readings and servo angles 4× per second. |
| `FLIP_PAN_PAST_FLAT` | `true` | Corrects the pan direction when tilted past flat (leave on). |
| `WRAP_PAN_AT_LIMITS` | `true` | Swings to the mirror-image pose when pan hits an end stop. |

---

## Troubleshooting

| Problem | Likely cause / fix |
|---------|--------------------|
| Moves **away** from the light on one axis | Flip `TILT_REVERSED` or `PAN_REVERSED` (see direction test). |
| Moves away on **both** axes, or wanders randomly | Readings are inverted. Run the sensor test and check `INVERT_LDR_READINGS`. |
| Moves the wrong axis (tilts when it should turn) | Sensors are in the wrong pins, or the `+` wall isn't lined up with the tilt axis. |
| Pan direction is only wrong when tilted to one side | `TILT_FLAT_ANGLE` doesn't match the angle where your panel is actually flat. |
| Shakes back and forth when pointed at the light | Raise `DEADBAND` (try `0.08`) or lower `GAIN`. |
| Arduino resets / servos twitch randomly | Servo power problem. Add the 100 µF capacitor or use a separate 5V supply with a shared GND. |
| Never moves | Too dark (`DARK-HOLD` in Serial Monitor), or `SENSOR_TEST_MODE` is still `true`. |
| Serial Monitor shows garbage | Set the baud rate to 9600. |
