# Hole-by-Hole Wiring Guide (Spaced-Out Layout)

This matches the pins in the code (`src/main.cpp` / `SolarTracker.ino`).
**Keep the USB cable unplugged while wiring.**

The layout uses **both halves of the breadboard**:
- **Left half** (columns a–e + left rails) = the **4 light sensors**
- **Right half** (columns f–j + right rails) = the **2 servos**

Every block has **empty rows between it and the next one**, so nothing is crowded.

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
| any color | servo "docking ports" (3 per servo, see Step 4) | 6 |

Short on white? Gray or purple is fine. Just use the **same** color for every 5V jumper, and don't use red/yellow/blue/green for 5V.

---

## Step 0: Start clean

1. **Unplug the USB.**
2. **Pull everything out of the breadboard.** Leave the Arduino alone.
3. **Stand the breadboard up vertically** with **row 1 at the top**.
4. Look at the letters across the top:
   - **`a` on the left, `j` on the right:** follow this guide exactly.
   - **`j` on the left:** your board is labeled the other way. Swap every letter (**a↔j, b↔i, c↔h, d↔g, e↔f**) and swap **"left rails" ↔ "right rails"**.
5. Both edges have two rails: **red line = `+`**, **blue/black line = `−`**. Go by the **color**, not which one is on the outside.
6. In the code, set **`SERVO_CENTER_MODE = true`** and upload (USB plugged in just for the upload, then unplug again). This holds both servos still at 90° while you wire and test, with no scanning.

> Reminder: in each row, **a–e are connected to each other**, and **f–j are connected to each other**, but the two halves are **not** connected across the middle groove.

---

## Layout maps

`═══` white 5V jumper, `───` black GND jumper, `~~~` 10 kΩ resistor.

### Left half: sensors

```
          LEFT RAILS   │
 Row      −       +    │   a                  c                     e
 ──────────────────────┼─────────────────────────────────────────────────────────
  1     [BLACK] [WHITE]│                                                ◄ from Arduino GND / 5V
  2                    │
  3              ●═════╪══ ● white                                ●    ◄ YELLOW sensor wire
  4      ●~~~~~~~~~~~~~╪~~ ● 10k           ● YELLOW jumper → A0    ●    ◄ YELLOW sensor wire
  5-7                  │                 (empty)
  8              ●═════╪══ ● white                                ●    ◄ RED sensor wire
  9      ●~~~~~~~~~~~~~╪~~ ● 10k           ● RED jumper → A1       ●    ◄ RED sensor wire
 10-12                 │                 (empty)
 13              ●═════╪══ ● white                                ●    ◄ BLUE sensor wire
 14      ●~~~~~~~~~~~~~╪~~ ● 10k           ● BLUE jumper → A2      ●    ◄ BLUE sensor wire
 15-17                 │                 (empty)
 18              ●═════╪══ ● white                                ●    ◄ GREEN sensor wire
 19      ●~~~~~~~~~~~~~╪~~ ● 10k           ● GREEN jumper → A3     ●    ◄ GREEN sensor wire
```

### Right half: servos

```
                                                            │   RIGHT RAILS
 Row     f                        h                    j    │    +       −
 ───────────────────────────────────────────────────────────┼──────────────────
  1                                                         │ [WHITE]          ◄ bridge from LEFT + rail
  2                                                         │         [BLACK]  ◄ bridge from LEFT − rail
  3-5                         (empty)                       │
  6      ● PAN brown                                   ●────┼─────────●        black jumper
  7      ● PAN red                                     ●════╪═══●              white jumper
  8      ● PAN orange             ● ORANGE → pin 9          │
  9-11                        (empty)                       │
 12                                                         │ [cap +] [cap −]  ◄ capacitor
 13-15                        (empty)                       │
 16      ● TILT brown                                  ●────┼─────────●        black jumper
 17      ● TILT red                                    ●════╪═══●              white jumper
 18      ● TILT orange            ● ORANGE → pin 10         │
```

(The maps show one rail order. If your board has + and − the other way around, that's fine. Just match the colors.)

All the cables that go up to the tracker (8 sensor wires + 2 servo plugs) land in **columns e and f**, right next to the center groove, so they all leave the board down the middle.

---

## Step 1: Power from the Arduino

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 1 | **white** jumper | Arduino **5V** | **left** red `+` rail, beside row 1 |
| 2 | **black** jumper | Arduino **GND** | **left** blue `−` rail, beside row 1 |

## Step 2: Bridges to the right rails

These two long jumpers carry power across the top of the board to the servo side.

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 3 | **white** jumper (long) | **left** red `+` rail, beside row 1 | **right** red `+` rail, beside row 1 |
| 4 | **black** jumper (long) | **left** blue `−` rail, beside row 2 | **right** blue `−` rail, beside row 2 |

> Double-check: **+ to +** and **− to −**. Crossing these shorts 5V to GND.

---

## Step 3: Sensor blocks (left half)

Do one block at a time. **All four are identical**, just 5 rows further down each time.

10 kΩ color bands: **brown-black-orange-gold** (4-band) or **brown-black-black-red-brown** (5-band). Resistors have no direction.

### Top-Left: YELLOW (rows 3–4)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 5 | **white** jumper | **a3** | left `+` rail, beside row 3 |
| 6 | 10 kΩ resistor | **a4** | left `−` rail, beside row 4 |
| 7 | **yellow** jumper | **c4** | Arduino **A0** |
| 8 | **yellow** sensor wire | **e3** | (photoresistor) |
| 9 | **yellow** sensor wire | **e4** | (photoresistor) |

### Top-Right: RED (rows 8–9)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 10 | **white** jumper | **a8** | left `+` rail, beside row 8 |
| 11 | 10 kΩ resistor | **a9** | left `−` rail, beside row 9 |
| 12 | **red** jumper | **c9** | Arduino **A1** |
| 13 | **red** sensor wire | **e8** | (photoresistor) |
| 14 | **red** sensor wire | **e9** | (photoresistor) |

### Bottom-Left: BLUE (rows 13–14)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 15 | **white** jumper | **a13** | left `+` rail, beside row 13 |
| 16 | 10 kΩ resistor | **a14** | left `−` rail, beside row 14 |
| 17 | **blue** jumper | **c14** | Arduino **A2** |
| 18 | **blue** sensor wire | **e13** | (photoresistor) |
| 19 | **blue** sensor wire | **e14** | (photoresistor) |

### Bottom-Right: GREEN (rows 18–19)

| # | Wire / part | End 1 | End 2 |
|---|-------------|-------|-------|
| 20 | **white** jumper | **a18** | left `+` rail, beside row 18 |
| 21 | 10 kΩ resistor | **a19** | left `−` rail, beside row 19 |
| 22 | **green** jumper | **c19** | Arduino **A3** |
| 23 | **green** sensor wire | **e18** | (photoresistor) |
| 24 | **green** sensor wire | **e19** | (photoresistor) |

In every block: the **upper** row has the white jumper + one sensor wire; the **lower** row has the resistor + colored jumper + the other sensor wire.

### ✅ Checkpoint: test the sensors now
Plug in USB and open the Serial Monitor (9600 baud). With `SERVO_CENTER_MODE = true` you'll see `MANUAL | TL:... TR:... BL:... BR:...` lines.
- Cover the **yellow** sensor: only **TL** should drop. **Red** → **TR**, **blue** → **BL**, **green** → **BR**.
- Shine a light on one: its number should go **up**.

Unplug USB before continuing.

---

## Step 4: Servo blocks (right half)

Each servo gets a 3-row **docking port**: push 3 male-to-male jumpers into the servo's plug, then push their other ends into column **f**, in order **brown, red, orange** from top to bottom.

> If your kit has a **male pin header strip**, snap off 3 pins, push them into f6–f8 (and f16–f18), and plug the servos straight onto them. Even neater.

Servo wire colors: **brown = GND**, **red = 5V**, **orange = signal** (some servos use black / red / yellow or white).

### Pan servo (base, turns clockwise/counterclockwise), rows 6–8

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 25 | docking jumper | servo plug **brown** | **f6** |
| 26 | docking jumper | servo plug **red** | **f7** |
| 27 | docking jumper | servo plug **orange** | **f8** |
| 28 | **black** jumper | **j6** | **right** `−` rail, beside row 6 |
| 29 | **white** jumper | **j7** | **right** `+` rail, beside row 7 |
| 30 | **orange** jumper | **h8** | Arduino pin **~9** |

### Capacitor, row 12

| # | Part | End 1 | End 2 |
|---|------|-------|-------|
| 31 | 100 µF capacitor | **long leg (+)** → **right** `+` rail, beside row 12 | **short leg (−)**, stripe side → **right** `−` rail, beside row 12 |

> ⚠️ The capacitor **has a direction**. Backwards, it can pop.

### Tilt servo (tips the panel plate), rows 16–18

| # | Wire | End 1 | End 2 |
|---|------|-------|-------|
| 32 | docking jumper | servo plug **brown** | **f16** |
| 33 | docking jumper | servo plug **red** | **f17** |
| 34 | docking jumper | servo plug **orange** | **f18** |
| 35 | **black** jumper | **j16** | **right** `−` rail, beside row 16 |
| 36 | **white** jumper | **j17** | **right** `+` rail, beside row 17 |
| 37 | **orange** jumper | **h18** | Arduino pin **~10** |

In every servo block: **brown row → black jumper → `−`**, **red row → white jumper → `+`**, **orange row → orange jumper → Arduino pin**.

### ✅ Checkpoint: test the servos now
Plug in USB, open the Serial Monitor, then type each line and press Enter:
- `p60`, then `p120`: the **pan** servo should turn.
- `t60`, then `t120`: the **tilt** servo should tip the plate.

If one doesn't move:
1. **Listen.** Buzzing = jammed or overloaded (unplug, check the plate moves freely by hand). Arm spins but plate doesn't = tighten the arm's screw. Silent = keep going.
2. **Swap test:** unplug USB, move that servo's three wires to the *other* servo's docking port, plug in, and type the *other* letter (`p60` instead of `t60`). If it moves there, the servo is fine and the problem is in its original rows. If it still doesn't move, the servo itself is bad.

---

## Step 5: Finish the setup

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
- [ ] Rows 3–4 yellow → A0, rows 8–9 red → A1, rows 13–14 blue → A2, rows 18–19 green → A3
- [ ] Sensor blocks: white jumper in the **upper** row, resistor + colored jumper in the **lower** row
- [ ] Servo plugs in column **f**, order **brown, red, orange** from top to bottom
- [ ] Pan: black **j6**, white **j7**, orange **h8 → pin 9**
- [ ] Tilt: black **j16**, white **j17**, orange **h18 → pin 10**
- [ ] Capacitor long leg on the **right `+`** rail
- [ ] Nothing in the empty rows, and nothing in columns **b, d, g, i**

---

## Common mistakes

| Symptom | Likely wiring mistake |
|---------|-----------------------|
| Nothing works at all | 5V/GND swapped, or the board is upside down (row 1 must be at the top) |
| Sensors work but no servo moves | Missing or crossed bridge jumpers (Step 2): the right rails have no power |
| A sensor always reads ~1023 | That block's resistor isn't reaching the left `−` rail |
| A sensor always reads ~0 | That block's white jumper isn't reaching the left `+` rail, or a sensor wire is loose |
| Covering one sensor changes a different number | Sensor wires are in the wrong color's block |
| One servo doesn't move | Its orange jumper is in the wrong row or pin (must be the **orange** row: h8 / h18), or its plug order is wrong |
| Servo twitches or Arduino resets when moving | Check the capacitor; if it still happens, power the right rails from a separate 5V supply (4×AA) **with its `−` connected to the Arduino GND**, and remove the Step 2 white `+` bridge |
