# Hole-by-Hole Wiring Guide (Neat Layout)

This matches the pins in the code (`src/main.cpp` / `SolarTracker.ino`).
**Keep the USB cable unplugged while wiring.**

---

## The idea: every column and every row block has one job

| Column | Job |
|--------|-----|
| **a** | Short connections to the **power rails** (jumpers and resistors) |
| **c** | Jumpers that go to the **Arduino** (signal wires) |
| **e** | **Cables that go up to the tracker** (LDR wires, servo plugs) |

| Rows | Job |
|------|-----|
| 1 | Power coming in from the Arduino |
| 3–4 | Top-Left sensor |
| 6–7 | Top-Right sensor |
| 9–10 | Bottom-Left sensor |
| 12–13 | Bottom-Right sensor |
| 17–19 | Pan servo |
| 20 | Capacitor (on the rails) |
| 21–23 | Tilt servo |
| 24–30 | Empty, free for anything you add later |

Every sensor block looks the same and every servo block looks the same. Empty rows between blocks give your fingers room. If something isn't working, you can check one block at a time.

---

## Get your bearings

- **Breadboard holes** are named by a **letter (a–j)** and a **row number** printed on the board. `c4` = column **c**, row **4**.
- In each row, holes **a b c d e** are connected together. The groove down the middle separates them from **f–j**, so nothing in this guide uses f–j.
- The long strips along the edge next to column **a** are the **power rails**. Every hole along a strip is connected. Go by the **printed colors**: **red line = `+` (5V)**, **blue/black line = `−` (GND)**. Your board may have them in either order.
- "The rail hole beside row 4" means the rail hole closest to row 4. Rail holes come in groups of 5 with small gaps, so just use the nearest one.
- Everything fits in **rows 1–30**, so it works on the small (400-hole) or big (830-hole) kit breadboard.

**Placement:** set the Arduino at the **row 1 end** of the breadboard. Power and sensor jumpers are then short. Only the two servo signal jumpers need to be longer.

---

## Layout map

The breadboard is drawn sideways: **each line of text is one row**.
`───` = jumper wire, `~~~` = 10 kΩ resistor, `●` = a hole that's used.

```
            POWER RAILS  │          MAIN AREA
 Row        −       +    │   a              c                 e
 ────────────────────────┼──────────────────────────────────────────────────────
  1       [GND]   [5V]   │                                          ◄ from Arduino GND / 5V
  2                      │
  3                ●─────┼── ●                                 ●   ◄ TL LDR wire 1
  4         ●~~~~~~~~~~~~┼~~ ●              ● ──► A0           ●   ◄ TL LDR wire 2
  5                      │
  6                ●─────┼── ●                                 ●   ◄ TR LDR wire 1
  7         ●~~~~~~~~~~~~┼~~ ●              ● ──► A1           ●   ◄ TR LDR wire 2
  8                      │
  9                ●─────┼── ●                                 ●   ◄ BL LDR wire 1
 10         ●~~~~~~~~~~~~┼~~ ●              ● ──► A2           ●   ◄ BL LDR wire 2
 11                      │
 12                ●─────┼── ●                                 ●   ◄ BR LDR wire 1
 13         ●~~~~~~~~~~~~┼~~ ●              ● ──► A3           ●   ◄ BR LDR wire 2
 14-16                   │                (empty)
 17         ●────────────┼── ●                                 ●   ◄ PAN servo brown
 18                ●─────┼── ●                                 ●   ◄ PAN servo red
 19                      │                  ● ──► pin 9        ●   ◄ PAN servo orange
 20      [cap −] [cap +] │
 21         ●────────────┼── ●                                 ●   ◄ TILT servo brown
 22                ●─────┼── ●                                 ●   ◄ TILT servo red
 23                      │                  ● ──► pin 10       ●   ◄ TILT servo orange
 24-30                   │                (empty: room to grow)
```

Each sensor block is a voltage divider: **+ rail → row 3 → LDR → row 4 → 10 kΩ → − rail**, with row 4 jumped to an analog pin. More light gives a higher reading, which is what the code expects.

---

## Suggested jumper colors

Pick colors once and stick to them so anyone can read the board at a glance:

| Color | Used for |
|-------|----------|
| **Red** | anything at 5V |
| **Black** (or brown) | anything at GND |
| **Yellow** | sensor signals to A0–A3 |
| **Orange** (or white) | servo signals to pins 9 and 10 |

Use the **shortest jumper that reaches** and lay it flat. Bend resistor legs so the resistor lies flat across from column a to the rail.

---

## Step 1: Power from the Arduino (row 1)

| # | From (Arduino) | To (breadboard) | Jumper |
|---|----------------|-----------------|--------|
| 1 | **5V** pin (POWER header) | **`+` rail** hole beside row 1 | red |
| 2 | **GND** pin (next to 5V) | **`−` rail** hole beside row 1 | black |

> Use **5V**, not 3.3V or VIN.

---

## Step 2: Sensor blocks (rows 3–13)

Do one block at a time. **All four blocks are identical**, just shifted down by 3 rows each.

10 kΩ color bands: **brown-black-orange-gold** (4-band) or **brown-black-black-red-brown** (5-band). Resistors have no direction.

### Top-Left sensor (rows 3–4)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 3 | red jumper | **a3** | `+` rail beside row 3 |
| 4 | 10 kΩ resistor | **a4** | `−` rail beside row 4 |
| 5 | yellow jumper | **c4** | Arduino **A0** |
| 6 | TL LDR wire 1 | **e3** | goes up to the tracker |
| 7 | TL LDR wire 2 | **e4** | goes up to the tracker |

### Top-Right sensor (rows 6–7)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 8  | red jumper | **a6** | `+` rail beside row 6 |
| 9  | 10 kΩ resistor | **a7** | `−` rail beside row 7 |
| 10 | yellow jumper | **c7** | Arduino **A1** |
| 11 | TR LDR wire 1 | **e6** | tracker |
| 12 | TR LDR wire 2 | **e7** | tracker |

### Bottom-Left sensor (rows 9–10)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 13 | red jumper | **a9** | `+` rail beside row 9 |
| 14 | 10 kΩ resistor | **a10** | `−` rail beside row 10 |
| 15 | yellow jumper | **c10** | Arduino **A2** |
| 16 | BL LDR wire 1 | **e9** | tracker |
| 17 | BL LDR wire 2 | **e10** | tracker |

### Bottom-Right sensor (rows 12–13)

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 18 | red jumper | **a12** | `+` rail beside row 12 |
| 19 | 10 kΩ resistor | **a13** | `−` rail beside row 13 |
| 20 | yellow jumper | **c13** | Arduino **A3** |
| 21 | BR LDR wire 1 | **e12** | tracker |
| 22 | BR LDR wire 2 | **e13** | tracker |

> Photoresistors have **no polarity**: either of an LDR's two wires can be "wire 1".
> Just keep **both wires of one LDR in its own block**. Never split one LDR across two blocks.
>
> "Top/Bottom" must be on opposite sides of the tilt axis, and "Left/Right" on opposite sides of the line across it. See *Sensor placement* in `README.md`.

**Wire colors (fill in so your team knows which cable is which):**

| Sensor | Block rows | Wire colors |
|--------|-----------|-------------|
| Top-Left | 3–4 | ______ / ______ |
| Top-Right | 6–7 | ______ / ______ |
| Bottom-Left | 9–10 | ______ / ______ |
| Bottom-Right | 12–13 | ______ / ______ |

---

## Step 3: Servo blocks (rows 17–23)

Each servo gets a 3-row **"docking port"**. Push 3 male-to-male jumpers into the servo's plug, then push those into column **e**, in order. The servo can later be unplugged and plugged back in without touching anything else.

> Even neater: if your kit has a **male pin header strip**, snap off 3 pins, push them into e17–e19, and plug the servo straight onto them. Do the same for e21–e23.

Servo wire colors: **brown = GND**, **red = 5V**, **orange = signal** (some servos use black / red / yellow or white).

### Pan servo (base, turns the tracker clockwise/counterclockwise), rows 17–19

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 23 | servo **brown** | **e17** | — |
| 24 | servo **red** | **e18** | — |
| 25 | servo **orange** | **e19** | — |
| 26 | black jumper | **a17** | `−` rail beside row 17 |
| 27 | red jumper | **a18** | `+` rail beside row 18 |
| 28 | orange jumper | **c19** | Arduino digital pin **~9** |

### Tilt servo (tips the panel plate up/down), rows 21–23

| # | Part | Hole | Other end |
|---|------|------|-----------|
| 29 | servo **brown** | **e21** | — |
| 30 | servo **red** | **e22** | — |
| 31 | servo **orange** | **e23** | — |
| 32 | black jumper | **a21** | `−` rail beside row 21 |
| 33 | red jumper | **a22** | `+` rail beside row 22 |
| 34 | orange jumper | **c23** | Arduino digital pin **~10** |

> Pins 9 and 10 are on the **"DIGITAL (PWM~)"** header, marked with a `~`.

---

## Step 4: 100 µF capacitor (row 20, between the servos)

This keeps servo power steady so the Arduino doesn't reset when both servos move.

| # | Capacitor leg | Hole |
|---|---------------|------|
| 35 | **long leg (+)** | **`+` rail** beside row 20 |
| 36 | **short leg (−)**, the stripe side | **`−` rail** beside row 20 |

> ⚠️ This capacitor **has a direction**: long leg to **red +**, stripe to **blue −**. Backwards, it can get hot or pop.

---

## Step 5: Cable management (where neatness really counts)

The tracker moves, so loose cables snag and pull out. To prevent that:

1. **Label each cable** near its breadboard end with a small masking-tape flag: `TL`, `TR`, `BL`, `BR`, `PAN`, `TILT`.
2. **Twist each LDR's two wires together** loosely, so each sensor is one tidy cable.
3. **Bundle** all six cables (4 sensors + 2 servos) with tape, twist ties or zip ties every few inches.
4. **Leave slack.** Before tying anything down, move the tracker by hand through its **full** pan and tilt range and make sure nothing pulls tight.
5. **Route cables close to the pan axis** (the center of rotation), so they twist a little instead of swinging around.
6. **Fix the boards down.** The breadboard has a sticky back; stick it and the Arduino side by side on a piece of cardboard or foam board, with the Arduino at the row 1 end.

---

## Step 6: Check before plugging in USB

- [ ] Arduino **5V → `+` rail**, **GND → `−` rail** (not swapped!)
- [ ] Column **a**: only red jumpers, black jumpers and resistors going to the rails
- [ ] Column **c**: only jumpers going to the Arduino (A0, A1, A2, A3, 9, 10)
- [ ] Column **e**: only the tracker's cables (8 LDR wires, 2 servo plugs)
- [ ] In each sensor block: the **upper** row has the red jumper and the **lower** row has the resistor plus the yellow jumper
- [ ] Both wires of each LDR are in **its own** block
- [ ] Servo plugs are in order: **brown, red, orange** going down (e17–e19 and e21–e23)
- [ ] Capacitor long leg on **red +**
- [ ] Nothing in columns f–j

Then plug in USB and run the **Sensor test** in `README.md` (`SENSOR_TEST_MODE = true`). Covering each sensor should make only its own number drop.

---

## Common mistakes

| Symptom | Likely wiring mistake |
|---------|-----------------------|
| A sensor always reads ~1023 | That block's resistor isn't reaching the `−` rail |
| A sensor always reads ~0 | That block's red jumper isn't reaching the `+` rail, or an LDR wire is loose in column e |
| A sensor reads random, jumpy numbers | Its yellow jumper is loose or in the wrong row (it must be in the **lower** row of the block) |
| Covering one sensor changes a different number | LDR cables are in the wrong blocks; check your tape labels |
| Readings go **down** with more light | Red jumper and resistor swapped (red jumper belongs in the upper row) |
| Servo doesn't move at all | Plug is in the wrong order, or the orange jumper isn't on the right pin |
| Servo twitches or Arduino resets when moving | Add/check the capacitor; if it still happens, power the servos from a separate 5V supply with its GND tied to the `−` rail |
