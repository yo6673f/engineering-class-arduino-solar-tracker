# Hole-by-Hole Wiring Guide (Spaced-Out Layout)

This matches the pins in the code (`src/main.cpp` / `SolarTracker.ino`).
**Keep the USB cable unplugged while wiring.**

The layout uses **both halves of the breadboard**:
- **Left half** (columns a–e + left rails) = the **2 servos** (and Arduino power comes in here)
- **Right half** (columns f–j + right rails) = the **4 light sensors**

Every block has **empty rows between it and the next one**, so nothing is crowded.

---

## How the power rails work

A rail is **not one hole**. It's the long column of holes running down the edge of the board, and **every hole in that column is connected**. So many wires can go into the same rail, each in **its own hole**.

```
 LEFT RAILS
  −    +         <- the + rail is the whole column under the red line
  o    *  <- Arduino 5V (white) goes here
  o    *  <- the bridge to the right side (white) goes in the next hole down
  o    o
  o    o  <- all of these are connected to each other
```

"**Left `+` rail, beside row 8**" means: any free hole in the left `+` rail, near row 8 (so the wire stays short). Always stay in the right rail: **red line = `+`**, **blue/black line = `−`**.

---

## Our photoresistor wire colors

Each photoresistor has **two wires of the same color**. They're interchangeable (photoresistors have no + or − side).

| Sensor (position on the tracker) | Wire color | Breadboard rows | Arduino pin |
|----------------------------------|-----------|-----------------|-------------|
| **Top-Left**     | **Yellow** | 3–4   | A0 |
| **Top-Right**    | **Red**    | 8–9   | A1 |
| **Bottom-Left**  | **Blue**   | 13–14 | A2 |
| **Bottom-Right** | **Green**  | 18–19 | A3 |

The photoresistor wires plug **straight into the breadboard**, no extra jumper needed. Strip about ¼ inch; if the wire is stranded, twist it tight (and tin it with solder if you can).

---

## Jumper colors

**A jumper's color tells you what it carries:**

| Jumper color | Carries | How many |
|--------------|---------|----------|
| **White** | 5V | 8 |
| **Black** | GND | 4 |
| **Yellow / Red / Blue / Green** | each sensor's signal (matches that sensor's wire color) | 1 each |
| **Orange** | servo signals → pins 9 and 10 | 2 |
| any color | servo "docking ports" (3 per servo, see Step 3) | 6 |

Short on white? Gray or purple is fine. Just use the **same** color for every 5V jumper, and don't use red/yellow/blue/green for 5V.

---

## Step 0: Start clean

1. **Unplug the USB.**
2. **Pull everything out of the breadboard.** Leave the Arduino alone.
3. **Stand the breadboard up vertically** with **row 1 at the top**.
4. Look at the letters across the top:
   - **`a` on the left, `j` on the right:** follow this guide exactly.
   - **`j` on the left:** your board is labeled the other way. Swap every letter (**a↔j, b↔i, c↔h, d↔g, e↔f**) and swap **"left rails" ↔ "right rails"**.
5. In the code, set **`SERVO_CENTER_MODE = true`** and upload (plug in USB just for the upload, then unplug again). This holds both servos still at 90° while you wire and test, with no scanning.

> Reminder: in each row, **a–e are connected to each other**, and **f–j are connected to each other**, but the two halves are **not** connected across the middle groove.

---

## Layout maps

`===` white 5V jumper, `---` black GND jumper, `~~~` 10 kΩ resistor.

### Left half: power + servos

```
          LEFT RAILS   |
 Row      −       +    |   a                  c                     e
 ----------------------+---------------------------------------------------------
  1     [BLACK] [WHITE]|                                                <- from Arduino GND / 5V
        (+ next holes: the two bridges to the right rails)
  2-5                  |                 (empty)
  6      *-------------+-- * black                                *    <- PAN servo brown
  7              *=====+== * white                                *    <- PAN servo red
  8                    |                   * ORANGE -> pin 9       *    <- PAN servo orange
  9-11                 |                 (empty)
 12     [cap -] [cap +]|                                                <- capacitor
 13-15                 |                 (empty)
 16      *-------------+-- * black                                *    <- TILT servo brown
 17              *=====+== * white                                *    <- TILT servo red
 18                    |                   * ORANGE -> pin 10      *    <- TILT servo orange
```

### Right half: sensors

```
                                                                 |   RIGHT RAILS
 Row     f                       h                       j       |    +       −
 ----------------------------------------------------------------+------------------
  1                                                              | [WHITE]           <- bridge from LEFT +
  2                                                              |         [BLACK]   <- bridge from LEFT −
  3      * YELLOW sensor wire                            * white=+====*
  4      * YELLOW sensor wire    * YELLOW -> A0          * 10k ~~+~~~~~~~~~*
  5-7                       (empty)                              |
  8      * RED sensor wire                               * white=+====*
  9      * RED sensor wire       * RED -> A1             * 10k ~~+~~~~~~~~~*
 10-12                      (empty)                              |
 13      * BLUE sensor wire                              * white=+====*
 14      * BLUE sensor wire      * BLUE -> A2            * 10k ~~+~~~~~~~~~*
 15-17                      (empty)                              |
 18      * GREEN sensor wire                             * white=+====*
 19      * GREEN sensor wire     * GREEN -> A3           * 10k ~~+~~~~~~~~~*
```

(The maps show one rail order. If your board has + and − the other way around, that's fine. Just match the colors.)

All the cables that go up to the tracker (2 servo plugs + 8 sensor wires) land in **columns e and f**, right next to the center groove, so they all leave the board down the middle.

---

## Step 1: Power from the Arduino

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 1 | **white** jumper | Arduino **5V** | **left** red `+` rail, beside row 1 |
| 2 | **black** jumper | Arduino **GND** | **left** blue `−` rail, beside row 1 |

## Step 2: Bridges to the right rails

These two long jumpers carry power across the top of the board to the sensor side. Each one goes in the **next free hole** of the left rail (the Arduino's wire is already in the first one).

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 3 | **white** jumper (long) | **left** red `+` rail, next free hole near row 1 | **right** red `+` rail, beside row 1 |
| 4 | **black** jumper (long) | **left** blue `−` rail, next free hole near row 2 | **right** blue `−` rail, beside row 2 |

> Double-check: **+ to +** and **− to −**. Crossing these shorts 5V to GND.

---

## Step 3: Servo blocks (left half)

Each servo gets a 3-row **docking port**: push 3 male-to-male jumpers into the servo's plug, then push their other ends into column **e**, in order **brown, red, orange** from top to bottom.

> If your kit has a **male pin header strip**, snap off 3 pins, push them into e6–e8 (and e16–e18), and plug the servos straight onto them. Even neater.

Servo wire colors: **brown = GND**, **red = 5V**, **orange = signal** (some servos use black / red / yellow or white).

### Pan servo (base, turns clockwise/counterclockwise), rows 6–8

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 5  | docking jumper | servo plug **brown** | **e6** |
| 6  | docking jumper | servo plug **red** | **e7** |
| 7  | docking jumper | servo plug **orange** | **e8** |
| 8  | **black** jumper | **a6** | **left** `−` rail, beside row 6 |
| 9  | **white** jumper | **a7** | **left** `+` rail, beside row 7 |
| 10 | **orange** jumper | **c8** | Arduino pin **~9** |

### Capacitor, row 12

| # | Part | End 1 | End 2 |
|---|------|-------|-------|
| 11 | 100 µF capacitor | **long leg (+)** → **left** `+` rail, beside row 12 | **short leg (−)**, stripe side → **left** `−` rail, beside row 12 |

> ⚠️ The capacitor **has a direction**. Backwards, it can pop.

### Tilt servo (tips the panel plate), rows 16–18

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 12 | docking jumper | servo plug **brown** | **e16** |
| 13 | docking jumper | servo plug **red** | **e17** |
| 14 | docking jumper | servo plug **orange** | **e18** |
| 15 | **black** jumper | **a16** | **left** `−` rail, beside row 16 |
| 16 | **white** jumper | **a17** | **left** `+` rail, beside row 17 |
| 17 | **orange** jumper | **c18** | Arduino pin **~10** |

In every servo block: **brown row → black jumper → `−`**, **red row → white jumper → `+`**, **orange row → orange jumper → Arduino pin**.

---

## Step 4: Sensor blocks (right half)

Do one block at a time. **All four are identical**, just 5 rows further down each time.

10 kΩ color bands: **brown-black-orange-gold** (4-band) or **brown-black-black-red-brown** (5-band). Resistors have no direction.

### Top-Left: YELLOW (rows 3–4)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 18 | **white** jumper | **j3** | right `+` rail, beside row 3 |
| 19 | 10 kΩ resistor | **j4** | right `−` rail, beside row 4 |
| 20 | **yellow** jumper | **h4** | Arduino **A0** |
| 21 | **yellow** sensor wire | **f3** | (photoresistor) |
| 22 | **yellow** sensor wire | **f4** | (photoresistor) |

### Top-Right: RED (rows 8–9)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 23 | **white** jumper | **j8** | right `+` rail, beside row 8 |
| 24 | 10 kΩ resistor | **j9** | right `−` rail, beside row 9 |
| 25 | **red** jumper | **h9** | Arduino **A1** |
| 26 | **red** sensor wire | **f8** | (photoresistor) |
| 27 | **red** sensor wire | **f9** | (photoresistor) |

### Bottom-Left: BLUE (rows 13–14)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 28 | **white** jumper | **j13** | right `+` rail, beside row 13 |
| 29 | 10 kΩ resistor | **j14** | right `−` rail, beside row 14 |
| 30 | **blue** jumper | **h14** | Arduino **A2** |
| 31 | **blue** sensor wire | **f13** | (photoresistor) |
| 32 | **blue** sensor wire | **f14** | (photoresistor) |

### Bottom-Right: GREEN (rows 18–19)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 33 | **white** jumper | **j18** | right `+` rail, beside row 18 |
| 34 | 10 kΩ resistor | **j19** | right `−` rail, beside row 19 |
| 35 | **green** jumper | **h19** | Arduino **A3** |
| 36 | **green** sensor wire | **f18** | (photoresistor) |
| 37 | **green** sensor wire | **f19** | (photoresistor) |

In every block: the **upper** row has the white jumper + one sensor wire; the **lower** row has the resistor + colored jumper + the other sensor wire.

---

## Step 5: Test

### ✅ Sensors
Plug in USB and open the Serial Monitor (9600 baud). With `SERVO_CENTER_MODE = true` you'll see `MANUAL | TL:... TR:... BL:... BR:...` lines.
- Cover the **yellow** sensor: only **TL** should drop. **Red** → **TR**, **blue** → **BL**, **green** → **BR**.
- Shine a light on one: its number should go **up**.
- If **no** sensor works, check the two bridge jumpers (Step 2).

### ✅ Servos
Type each line and press Enter:
- `p60`, then `p120`: the **pan** servo should turn.
- `t60`, then `t120`: the **tilt** servo should tip the plate.

If one doesn't move:
1. **Listen.** Buzzing = jammed or overloaded (unplug, check the plate moves freely by hand). Arm spins but plate doesn't = tighten the arm's screw. Silent = keep going.
2. **Swap test:** unplug USB, move that servo's three wires to the *other* servo's docking port, plug in, and type the *other* letter (`p60` instead of `t60`). If it moves there, the servo is fine and the problem is in its original rows. If it still doesn't move, the servo itself is bad.

---

## Step 6: Finish the setup

1. With `SERVO_CENTER_MODE = true` still on, both servos are at 90°. **Re-seat the servo arms** so the tracker points straight ahead and the plate is flat (see README, "Center the servo arms").
2. Set `SERVO_CENTER_MODE = false`, upload, and follow the README from **"Find your servo limits"**.

---

## Cable management

1. Your sensor cables are already color-coded. Add masking-tape flags to the servo cables: `PAN`, `TILT`.
2. Twist each sensor's two same-color wires loosely together.
3. All tracker cables leave from **columns e and f** down the middle. **Bundle** them with tape or zip ties every few inches.
4. **Leave slack.** Move the tracker by hand through its **full** range before tying anything down.
5. Tape each sensor's wires to the `+` wall about an inch from the sensor, so a tug pulls on the tape and not the photoresistor's legs.

---

## Final check before plugging in USB

- [ ] Arduino **5V → left `+`**, **GND → left `−`**
- [ ] Bridges: **left `+` → right `+`** (white), **left `−` → right `−`** (black)
- [ ] Pan: plug in **e6–e8** (brown, red, orange), black **a6**, white **a7**, orange **c8 → pin 9**
- [ ] Tilt: plug in **e16–e18** (brown, red, orange), black **a16**, white **a17**, orange **c18 → pin 10**
- [ ] Capacitor long leg on the **left `+`** rail
- [ ] Sensors: rows 3–4 yellow → A0, rows 8–9 red → A1, rows 13–14 blue → A2, rows 18–19 green → A3
- [ ] Sensor blocks: white jumper in the **upper** row, resistor + colored jumper in the **lower** row
- [ ] Nothing in the empty rows, and nothing in columns **b, d, g, i**

---

## Common mistakes

| Symptom | Likely wiring mistake |
|---------|-----------------------|
| Nothing works at all | 5V/GND swapped, or the board is upside down (row 1 must be at the top) |
| Servos work but no sensor reads anything | Missing or crossed bridge jumpers (Step 2): the right rails have no power |
| A sensor always reads ~1023 | That block's resistor isn't reaching the right `−` rail |
| A sensor always reads ~0 | That block's white jumper isn't reaching the right `+` rail, or a sensor wire is loose |
| Covering one sensor changes a different number | Sensor wires are in the wrong color's block |
| One servo doesn't move | Its orange jumper is in the wrong row or pin (must be the **orange** row: c8 / c18), or its plug order is wrong |
| Servo twitches or Arduino resets when moving | Check the capacitor; if it still happens, power the servos from a separate 5V supply (4×AA): its `+` to the **left `+`** rail, its `−` to the **left `−`** rail, and **unplug the Arduino 5V wire (Step 1, #1)** but keep the GND wire |
