# Hole-by-Hole Wiring Guide (Neat Layout, Left Power Rails)

This matches the pins in the code (`src/main.cpp` / `SolarTracker.ino`).
**Keep the USB cable unplugged while wiring.**

---

## Our photoresistor wire colors

Each photoresistor has **two wires of the same color**. They're interchangeable because photoresistors have no + or − side.

| Sensor (position on the tracker) | Wire color | Breadboard rows | Arduino pin |
|----------------------------------|-----------|-----------------|-------------|
| **Top-Left**     | **Yellow** | 3–4   | A0 |
| **Top-Right**    | **Red**    | 6–7   | A1 |
| **Bottom-Left**  | **Blue**   | 9–10  | A2 |
| **Bottom-Right** | **Green**  | 12–13 | A3 |

---

## Do the photoresistor wires need an extra jumper?

**No.** The bare end of each photoresistor wire plugs **straight into its breadboard hole**. You only need to prep the ends:

1. Strip about **¼ inch (6 mm)** of insulation off the free end of each wire.
2. Check whether the wire is **solid core** (one stiff metal strand) or **stranded** (many thin strands):
   - **Solid core:** ready to go; push it straight into the hole.
   - **Stranded:** twist the strands tightly, then **tin** them (melt a little solder into the twisted strands) so the end is stiff like a pin. Do this away from the breadboard, and never solder onto the breadboard itself.
3. Only if a wire is **too thick to fit** in the hole, or too floppy even after tinning: cut a male-to-male jumper in half, solder the wire to the jumper's cut end, and cover the joint with heat-shrink or electrical tape. Then plug the jumper's pin into the hole instead.

---

## Jumper colors (so nothing gets confused with the sensor wires)

The rule is that **a jumper's color tells you what it carries**:

| Jumper color | Carries | How many |
|--------------|---------|----------|
| **White** | 5V | 7 |
| **Black** | GND | 3 |
| **Yellow** | Top-Left signal → A0 (matches the yellow sensor) | 1 |
| **Red** | Top-Right signal → A1 (matches the red sensor) | 1 |
| **Blue** | Bottom-Left signal → A2 (matches the blue sensor) | 1 |
| **Green** | Bottom-Right signal → A3 (matches the green sensor) | 1 |
| **Orange** | Servo signals → pins 9 and 10 (matches the servos' orange wire) | 2 |
| any color | Servo "docking ports" (3 per servo, see Step 4) | 6 |

> Short on white? Gray or purple works too. Just use the **same** color for every 5V jumper, and don't use red/yellow/blue/green for 5V.

All jumpers are **male-to-male**. The Arduino's pin headers are sockets, so a jumper end pushes straight into them.

---

## Step 0: Set up the breadboard

1. **Stand the breadboard up vertically** with **row 1 at the top**. Row numbers count **downward**.
2. We only use the **two power rails on the LEFT edge**. The right rails stay empty.
3. Go by the printed color, not position: **red line = `+` (5V)**, **blue/black line = `−` (GND)**.
4. Look at the letters across the top:
   - **Column `a` is on the left** (next to the left rails): follow this guide exactly.
   - **Column `j` is on the left**: swap letters everywhere: **a → j, c → h, e → f**.
5. Put the Arduino just above or beside the top-left corner of the breadboard.

**One job per column:**
- **a** (next to the left rails): connections to the rails
- **c** (middle): jumpers to the Arduino
- **e** (next to the center groove): cables that go up to the tracker

"Rail beside row 4" = the left-rail hole closest to row 4. Rail holes come in groups of 5 with small gaps, so just use the nearest one.

---

## Layout map

Exactly as you see it with the board standing up (row 1 at top, left rails on the left).

```
          LEFT RAILS     │
 Row      −       +      │   a                c                    e
 ────────────────────────┼──────────────────────────────────────────────────────
  1    [BLACK] [WHITE]   │                                              ◄ from Arduino GND / 5V
  2                      │
  3              ●═══════╪══ ● (white)                            ●    ◄ YELLOW sensor wire
  4      ●~~~~~~~~~~~~~~~╪~~ ● (10k)        ● YELLOW jumper → A0   ●    ◄ YELLOW sensor wire
  5                      │
  6              ●═══════╪══ ● (white)                            ●    ◄ RED sensor wire
  7      ●~~~~~~~~~~~~~~~╪~~ ● (10k)        ● RED jumper → A1      ●    ◄ RED sensor wire
  8                      │
  9              ●═══════╪══ ● (white)                            ●    ◄ BLUE sensor wire
 10      ●~~~~~~~~~~~~~~~╪~~ ● (10k)        ● BLUE jumper → A2     ●    ◄ BLUE sensor wire
 11                      │
 12              ●═══════╪══ ● (white)                            ●    ◄ GREEN sensor wire
 13      ●~~~~~~~~~~~~~~~╪~~ ● (10k)        ● GREEN jumper → A3    ●    ◄ GREEN sensor wire
 14-16                   │                (empty)
 17      ●───────────────┼── ● (black)                            ●    ◄ PAN servo brown
 18              ●═══════╪══ ● (white)                            ●    ◄ PAN servo red
 19                      │                  ● ORANGE jumper → 9    ●    ◄ PAN servo orange
 20    [cap −] [cap +]   │
 21      ●───────────────┼── ● (black)                            ●    ◄ TILT servo brown
 22              ●═══════╪══ ● (white)                            ●    ◄ TILT servo red
 23                      │                  ● ORANGE jumper → 10   ●    ◄ TILT servo orange
 24-30                   │                (empty: room to grow)
```

`═══` white 5V jumper, `───` black GND jumper, `~~~` 10 kΩ resistor.
(The map draws `−` on the outside and `+` on the inside; if your board is the other way around, just match the colors.)

---

## Step 1: Power from the Arduino

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 1 | **white** jumper | Arduino **5V** socket (POWER header) | **left red `+` rail**, beside row 1 |
| 2 | **black** jumper | Arduino **GND** socket (right next to 5V) | **left blue `−` rail**, beside row 1 |

---

## Step 2: Sensor blocks

10 kΩ color bands: **brown-black-orange-gold** (4-band) or **brown-black-black-red-brown** (5-band). Resistors have no direction.

### Top-Left: YELLOW (rows 3–4)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 3 | **white** jumper | **a3** | left red `+` rail, beside row 3 |
| 4 | 10 kΩ resistor | **a4** | left blue `−` rail, beside row 4 |
| 5 | **yellow** jumper | **c4** | Arduino **A0** |
| 6 | **yellow** sensor wire #1 | **e3** | (soldered to the photoresistor) |
| 7 | **yellow** sensor wire #2 | **e4** | (soldered to the photoresistor) |

### Top-Right: RED (rows 6–7)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 8  | **white** jumper | **a6** | left red `+` rail, beside row 6 |
| 9  | 10 kΩ resistor | **a7** | left blue `−` rail, beside row 7 |
| 10 | **red** jumper | **c7** | Arduino **A1** |
| 11 | **red** sensor wire #1 | **e6** | (photoresistor) |
| 12 | **red** sensor wire #2 | **e7** | (photoresistor) |

### Bottom-Left: BLUE (rows 9–10)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 13 | **white** jumper | **a9** | left red `+` rail, beside row 9 |
| 14 | 10 kΩ resistor | **a10** | left blue `−` rail, beside row 10 |
| 15 | **blue** jumper | **c10** | Arduino **A2** |
| 16 | **blue** sensor wire #1 | **e9** | (photoresistor) |
| 17 | **blue** sensor wire #2 | **e10** | (photoresistor) |

### Bottom-Right: GREEN (rows 12–13)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 18 | **white** jumper | **a12** | left red `+` rail, beside row 12 |
| 19 | 10 kΩ resistor | **a13** | left blue `−` rail, beside row 13 |
| 20 | **green** jumper | **c13** | Arduino **A3** |
| 21 | **green** sensor wire #1 | **e12** | (photoresistor) |
| 22 | **green** sensor wire #2 | **e13** | (photoresistor) |

> Both wires of a sensor are the same color and interchangeable; it doesn't matter which one is "#1".
> The only rule: **both wires of a color go in that color's block.**

Each block is a voltage divider: **+ rail → white jumper → upper row → photoresistor → lower row → 10 kΩ → − rail**, with the lower row jumped to the analog pin. More light gives a higher reading.

---

## Step 3: 100 µF capacitor (row 20)

| # | Part | End 1 | End 2 |
|---|------|-------|-------|
| 23 | 100 µF capacitor | **long leg (+)** → left red `+` rail, beside row 20 | **short leg (−)**, stripe side → left blue `−` rail, beside row 20 |

> ⚠️ This capacitor **has a direction**. Backwards, it can get hot or pop.

---

## Step 4: Servos

Each servo has a plug with 3 sockets: **brown (GND), red (5V), orange (signal)**.
First make a **docking port**: push 3 male-to-male jumpers into the 3 sockets of the servo's plug. Then push the jumpers' other ends into the breadboard. To unplug the servo later, just pull the plug off its jumpers.

> Even neater: if your kit has a **male pin header strip**, snap off 3 pins, push them into e17–e19, and plug the servo straight onto them (same for e21–e23). Then skip the docking-port jumpers.

### Pan servo (base, turns clockwise/counterclockwise), rows 17–19

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 24 | docking jumper | servo plug **brown** socket | **e17** |
| 25 | docking jumper | servo plug **red** socket | **e18** |
| 26 | docking jumper | servo plug **orange** socket | **e19** |
| 27 | **black** jumper | **a17** | left blue `−` rail, beside row 17 |
| 28 | **white** jumper | **a18** | left red `+` rail, beside row 18 |
| 29 | **orange** jumper | **c19** | Arduino digital **~9** |

### Tilt servo (tips the panel plate), rows 21–23

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 30 | docking jumper | servo plug **brown** socket | **e21** |
| 31 | docking jumper | servo plug **red** socket | **e22** |
| 32 | docking jumper | servo plug **orange** socket | **e23** |
| 33 | **black** jumper | **a21** | left blue `−` rail, beside row 21 |
| 34 | **white** jumper | **a22** | left red `+` rail, beside row 22 |
| 35 | **orange** jumper | **c23** | Arduino digital **~10** |

> Pins 9 and 10 are on the **"DIGITAL (PWM~)"** header, marked with a `~`.

---

## Step 5: Cable management

1. Your sensor cables are already color-coded. Add masking-tape flags to the servo cables: `PAN`, `TILT`.
2. Twist each sensor's two same-color wires loosely together, so each sensor is one cable.
3. Every tracker cable comes out of **column e**, in one straight line. **Bundle** them with tape or zip ties every few inches.
4. **Leave slack.** Before tying anything down, move the tracker by hand through its **full** pan and tilt range and make sure nothing pulls tight.
5. Route cables close to the **pan axis** (center of rotation).
6. Stick the breadboard (vertical, row 1 at top) and the Arduino near its top-left corner onto cardboard or foam board.

---

## Step 6: Check before plugging in USB

- [ ] **White** jumper from Arduino **5V** → left **red** rail; **black** from Arduino **GND** → left **blue** rail
- [ ] Nothing in the **right** rails or columns **f–j**
- [ ] Rows 3–4 = both **yellow** wires + yellow jumper to **A0**
- [ ] Rows 6–7 = both **red** wires + red jumper to **A1**
- [ ] Rows 9–10 = both **blue** wires + blue jumper to **A2**
- [ ] Rows 12–13 = both **green** wires + green jumper to **A3**
- [ ] In each sensor block: the **white** jumper is in the **upper** row; the **resistor** and **colored** jumper are in the **lower** row
- [ ] Servo plugs go **brown, red, orange** from top to bottom
- [ ] **Orange** jumpers: row 19 → pin **9**, row 23 → pin **10**
- [ ] Capacitor long leg on the **red** rail

Then plug in USB and run the **Sensor test** in `README.md` (`SENSOR_TEST_MODE = true`):
cover the **yellow** sensor and `TL` should drop, cover **red** and `TR` should drop, **blue** → `BL`, **green** → `BR`.

---

## Common mistakes

| Symptom | Likely wiring mistake |
|---------|-----------------------|
| Nothing works at all | 5V/GND went to the right rails, or the board is upside down (row 1 must be at the top) |
| A sensor always reads ~1023 | That block's resistor isn't reaching the left `−` rail |
| A sensor always reads ~0 | That block's white jumper isn't reaching the left `+` rail, or a sensor wire end is loose in column e |
| A sensor reads random, jumpy numbers | Its colored jumper is loose or in the wrong row (it must be in the **lower** row) |
| Covering one sensor changes a different number | Sensor wires are in the wrong color's block |
| Readings go **down** with more light | White jumper and resistor swapped (white belongs in the upper row) |
| A sensor wire keeps falling out | Its end is stranded or too short; strip ¼ inch and tin it |
| Servo doesn't move at all | Plug is in the wrong order, or the orange jumper isn't on the right pin |
| Servo twitches or Arduino resets when moving | Check the capacitor; if it still happens, power the servos from a separate 5V supply with its GND tied to the left `−` rail |
