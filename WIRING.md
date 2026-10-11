# Hole-by-Hole Wiring Guide (Neat Layout, Left Power Rails)

This matches the pins in the code (`src/main.cpp` / `SolarTracker.ino`).
**Keep the USB cable unplugged while wiring.**

---

## Step 0: Set up the breadboard the right way

1. **Stand the breadboard up vertically** (long way up and down) with **row 1 at the top**. The row numbers now count **downward**.
2. You'll see **two power rails running down the left edge** and two down the right edge. **We only use the two LEFT rails.** The right rails stay empty.
3. The two left rails have printed lines: **red line = `+` (5V)**, **blue/black line = `−` (GND)**. Which one is on the outside depends on the brand, so **always go by the color, not the position.**
4. Look at the letters printed across the top of the board:
   - **Column `a` is on the left (next to the left rails):** perfect, follow this guide exactly.
   - **Column `j` is on the left instead:** your board is labeled the other way. Everything still works; just swap the letters in every instruction:

     | Guide says | You use |
     |-----------|---------|
     | **a** | **j** |
     | **c** | **h** |
     | **e** | **f** |

5. Set the **Arduino just above or to the left of the top of the breadboard**, near row 1, so the power and sensor jumpers are short.

> In every row, the 5 holes on the left side of the center groove are connected together. The 5 holes on the right side are connected together too, but separately. This guide only uses the **left 5 holes (a–e)** of each row, right next to the left rails.

---

## The idea: every column and every row block has one job

| Column | Job |
|--------|-----|
| **a** (closest to the left rails) | Short connections to the **left power rails** (jumpers and resistors) |
| **c** (middle) | Jumpers that go to the **Arduino** (signal wires) |
| **e** (next to the center groove) | **Cables that go up to the tracker** (LDR wires, servo plugs) |

| Rows | Job |
|------|-----|
| 1 | Power coming in from the Arduino |
| 3–4 | Top-Left sensor |
| 6–7 | Top-Right sensor |
| 9–10 | Bottom-Left sensor |
| 12–13 | Bottom-Right sensor |
| 17–19 | Pan servo |
| 20 | Capacitor (on the left rails) |
| 21–23 | Tilt servo |
| 24–30 | Empty, free for anything you add later |

---

## Layout map

This is the top of your breadboard exactly as you see it standing up: **left rails on the left, row 1 at the top.**
`───` = jumper wire, `~~~` = 10 kΩ resistor, `●` = a hole that's used.
"Rail beside row 4" means the left-rail hole closest to row 4. Rail holes come in groups of 5 with small gaps, so just use the nearest one.

```
          LEFT RAILS      │
 Row      −       +       │   a              c                 e    ┃ groove ┃  f–j + right rails: unused
 ─────────────────────────┼─────────────────────────────────────────
  1     [GND]   [5V]      │                                            ◄ from Arduino GND / 5V
  2                       │
  3              ●────────┼── ●                                 ●     ◄ TL LDR wire 1
  4      ●~~~~~~~~~~~~~~~~┼~~ ●              ● ──► A0           ●     ◄ TL LDR wire 2
  5                       │
  6              ●────────┼── ●                                 ●     ◄ TR LDR wire 1
  7      ●~~~~~~~~~~~~~~~~┼~~ ●              ● ──► A1           ●     ◄ TR LDR wire 2
  8                       │
  9              ●────────┼── ●                                 ●     ◄ BL LDR wire 1
 10      ●~~~~~~~~~~~~~~~~┼~~ ●              ● ──► A2           ●     ◄ BL LDR wire 2
 11                       │
 12              ●────────┼── ●                                 ●     ◄ BR LDR wire 1
 13      ●~~~~~~~~~~~~~~~~┼~~ ●              ● ──► A3           ●     ◄ BR LDR wire 2
 14-16                    │                (empty)
 17      ●────────────────┼── ●                                 ●     ◄ PAN servo brown
 18              ●────────┼── ●                                 ●     ◄ PAN servo red
 19                       │                  ● ──► pin 9        ●     ◄ PAN servo orange
 20    [cap −] [cap +]    │
 21      ●────────────────┼── ●                                 ●     ◄ TILT servo brown
 22              ●────────┼── ●                                 ●     ◄ TILT servo red
 23                       │                  ● ──► pin 10       ●     ◄ TILT servo orange
 24-30                    │                (empty: room to grow)
```

(The map draws `−` on the outside and `+` on the inside. If your board is the other way around, that's fine: just match the colors.)

Each sensor block is a voltage divider: **left `+` rail → upper row → LDR → lower row → 10 kΩ → left `−` rail**, with the lower row jumped to an analog pin. More light gives a higher reading, which is what the code expects.

---

## Suggested jumper colors

| Color | Used for |
|-------|----------|
| **Red** | anything at 5V |
| **Black** (or brown) | anything at GND |
| **Yellow** | sensor signals to A0–A3 |
| **Orange** (or white) | servo signals to pins 9 and 10 |

Use the **shortest jumper that reaches** and lay it flat. Bend resistor legs so each resistor lies flat from column a over to the left rail.

---

## Step 1: Power from the Arduino (row 1)

| # | From (Arduino) | To (breadboard) | Jumper |
|---|----------------|-----------------|--------|
| 1 | **5V** pin (POWER header) | **left red `+` rail**, hole beside row 1 | red |
| 2 | **GND** pin (next to 5V) | **left blue `−` rail**, hole beside row 1 | black |

> Use **5V**, not 3.3V or VIN.

---

## Step 2: Sensor blocks (rows 3–13)

Do one block at a time. **All four blocks are identical**, just shifted down by 3 rows each.

10 kΩ color bands: **brown-black-orange-gold** (4-band) or **brown-black-black-red-brown** (5-band). Resistors have no direction.

### Top-Left sensor (rows 3–4)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 3 | red jumper | **a3** | left `+` rail beside row 3 |
| 4 | 10 kΩ resistor | **a4** | left `−` rail beside row 4 |
| 5 | yellow jumper | **c4** | Arduino **A0** |
| 6 | TL LDR wire 1 | **e3** | goes up to the tracker |
| 7 | TL LDR wire 2 | **e4** | goes up to the tracker |

### Top-Right sensor (rows 6–7)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 8  | red jumper | **a6** | left `+` rail beside row 6 |
| 9  | 10 kΩ resistor | **a7** | left `−` rail beside row 7 |
| 10 | yellow jumper | **c7** | Arduino **A1** |
| 11 | TR LDR wire 1 | **e6** | tracker |
| 12 | TR LDR wire 2 | **e7** | tracker |

### Bottom-Left sensor (rows 9–10)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 13 | red jumper | **a9** | left `+` rail beside row 9 |
| 14 | 10 kΩ resistor | **a10** | left `−` rail beside row 10 |
| 15 | yellow jumper | **c10** | Arduino **A2** |
| 16 | BL LDR wire 1 | **e9** | tracker |
| 17 | BL LDR wire 2 | **e10** | tracker |

### Bottom-Right sensor (rows 12–13)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 18 | red jumper | **a12** | left `+` rail beside row 12 |
| 19 | 10 kΩ resistor | **a13** | left `−` rail beside row 13 |
| 20 | yellow jumper | **c13** | Arduino **A3** |
| 21 | BR LDR wire 1 | **e12** | tracker |
| 22 | BR LDR wire 2 | **e13** | tracker |

> Photoresistors have **no polarity**: either of an LDR's two wires can be "wire 1".
> Just keep **both wires of one LDR in its own block**. Never split one LDR across two blocks.
>
> "Top/Bottom" must be on opposite sides of the tilt axis, and "Left/Right" on opposite sides of the line across it. See *Sensor placement* in `README.md`. (These are positions on the **tracker**, not on the breadboard.)

**Wire colors (fill in so your team knows which cable is which):**

| Sensor | Block rows | Wire colors |
|--------|-----------|-------------|
| Top-Left | 3–4 | ______ / ______ |
| Top-Right | 6–7 | ______ / ______ |
| Bottom-Left | 9–10 | ______ / ______ |
| Bottom-Right | 12–13 | ______ / ______ |

---

## Step 3: Servo blocks (rows 17–23)

Each servo gets a 3-row **"docking port"**. Push 3 male-to-male jumpers into the servo's plug, then push those into column **e**, in order from top to bottom. The servo can later be unplugged and plugged back in without touching anything else.

> Even neater: if your kit has a **male pin header strip**, snap off 3 pins, push them into e17–e19, and plug the servo straight onto them. Do the same for e21–e23.

Servo wire colors: **brown = GND**, **red = 5V**, **orange = signal** (some servos use black / red / yellow or white).

### Pan servo (base, turns the tracker clockwise/counterclockwise), rows 17–19

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 23 | servo **brown** | **e17** | — |
| 24 | servo **red** | **e18** | — |
| 25 | servo **orange** | **e19** | — |
| 26 | black jumper | **a17** | left `−` rail beside row 17 |
| 27 | red jumper | **a18** | left `+` rail beside row 18 |
| 28 | orange jumper | **c19** | Arduino digital pin **~9** |

### Tilt servo (tips the panel plate up/down), rows 21–23

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 29 | servo **brown** | **e21** | — |
| 30 | servo **red** | **e22** | — |
| 31 | servo **orange** | **e23** | — |
| 32 | black jumper | **a21** | left `−` rail beside row 21 |
| 33 | red jumper | **a22** | left `+` rail beside row 22 |
| 34 | orange jumper | **c23** | Arduino digital pin **~10** |

> Pins 9 and 10 are on the **"DIGITAL (PWM~)"** header, marked with a `~`.

---

## Step 4: 100 µF capacitor (row 20, between the servos)

This keeps servo power steady so the Arduino doesn't reset when both servos move.

| # | Capacitor leg | Hole |
|---|---------------|------|
| 35 | **long leg (+)** | **left red `+` rail** beside row 20 |
| 36 | **short leg (−)**, the stripe side | **left blue `−` rail** beside row 20 |

> ⚠️ This capacitor **has a direction**: long leg to **red +**, stripe to **blue −**. Backwards, it can get hot or pop.

---

## Step 5: Cable management

The tracker moves, so loose cables snag and pull out. To prevent that:

1. **Label each cable** near its breadboard end with a small masking-tape flag: `TL`, `TR`, `BL`, `BR`, `PAN`, `TILT`.
2. **Twist each LDR's two wires together** loosely, so each sensor is one tidy cable.
3. All the tracker cables come out of **column e**, so they leave the board in one neat line down the center groove. **Bundle** all six (4 sensors + 2 servos) with tape, twist ties or zip ties every few inches.
4. **Leave slack.** Before tying anything down, move the tracker by hand through its **full** pan and tilt range and make sure nothing pulls tight.
5. **Route cables close to the pan axis** (the center of rotation), so they twist a little instead of swinging around.
6. **Fix the boards down.** The breadboard has a sticky back; stick it (vertical, row 1 at the top) and the Arduino near its top-left corner onto a piece of cardboard or foam board.

---

## Step 6: Check before plugging in USB

- [ ] Arduino **5V → left red `+` rail**, **GND → left blue `−` rail** (not swapped!)
- [ ] Nothing is plugged into the **right** rails or columns **f–j**
- [ ] Column **a**: only red jumpers, black jumpers and resistors going to the left rails
- [ ] Column **c**: only jumpers going to the Arduino (A0, A1, A2, A3, 9, 10)
- [ ] Column **e**: only the tracker's cables (8 LDR wires, 2 servo plugs)
- [ ] In each sensor block: the **upper** row has the red jumper and the **lower** row has the resistor plus the yellow jumper
- [ ] Both wires of each LDR are in **its own** block
- [ ] Servo plugs go **brown, red, orange** from top to bottom (e17–e19 and e21–e23)
- [ ] Capacitor long leg on **red +**

Then plug in USB and run the **Sensor test** in `README.md` (`SENSOR_TEST_MODE = true`). Covering each sensor should make only its own number drop.

---

## Common mistakes

| Symptom | Likely wiring mistake |
|---------|-----------------------|
| Nothing works at all | 5V/GND went to the right rails, or the board is upside down (row 1 must be at the top) |
| A sensor always reads ~1023 | That block's resistor isn't reaching the left `−` rail |
| A sensor always reads ~0 | That block's red jumper isn't reaching the left `+` rail, or an LDR wire is loose in column e |
| A sensor reads random, jumpy numbers | Its yellow jumper is loose or in the wrong row (it must be in the **lower** row of the block) |
| Covering one sensor changes a different number | LDR cables are in the wrong blocks; check your tape labels |
| Readings go **down** with more light | Red jumper and resistor swapped (red jumper belongs in the upper row) |
| Servo doesn't move at all | Plug is in the wrong order, or the orange jumper isn't on the right pin |
| Servo twitches or Arduino resets when moving | Add/check the capacitor; if it still happens, power the servos from a separate 5V supply with its GND tied to the left `−` rail |
