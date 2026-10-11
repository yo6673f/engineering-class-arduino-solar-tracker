# Hole-by-Hole Wiring Guide

This matches the pins in the code (`src/main.cpp` / `SolarTracker.ino`).
**Keep the USB cable unplugged while wiring.**

---

## Get your bearings

- **Breadboard holes** are named by a **letter (a–j)** and a **row number** printed on the board. For example, `c5` = column **c**, row **5**.
- In each row, holes **a b c d e** are connected to each other, and so are **f g h i j**. The groove down the middle splits them, so `e5` and `f5` are **not** connected.
- The long strips along each edge are **power rails**. Every hole along one strip is connected, so the exact hole along the strip doesn't matter.
- Use the **pair of rails running alongside column `a`**. Go by the **printed colors**: **red line = `+` rail (5V)**, **blue/black line = `−` rail (GND)**.
- Everything below uses **rows 1–30 and columns a–e only**. This works on both the small (400-hole) and big (830-hole) kit breadboards. On the big board, some rails are split in the middle, and staying in rows 1–30 avoids that.

---

## Overview

```
 Breadboard row:      5            10           15           20          25-28
                      |            |            |            |            |
   e   ............ [jumper→A0] ..[jumper→A1] ..[jumper→A2] ..[jumper→A3] ......
   d   .............................................................................
   c   ............ [TL LDR wire] [TR LDR wire] [BL LDR wire] [BR LDR wire] .......
   b   .............................................................................
   a   ............ [10k leg] ... [10k leg] ... [10k leg] ... [10k leg] ...........
                      |            |            |            |
  −  (blue) ===== 10k leg ====== 10k leg ====== 10k leg ====== 10k leg == servo brown x2, cap −, Arduino GND
  +  (red)  ===== other wire of all 4 LDRs ====================== servo red x2, cap +, Arduino 5V
```

Each sensor row (5, 10, 15, 20) is a **voltage divider**:
`5V (+ rail) → LDR → row → 10kΩ → GND (− rail)`, with a jumper from that row to an analog pin.

---

## Step 1: Power the rails from the Arduino

| # | From (Arduino) | To (breadboard) | Suggested jumper color |
|---|----------------|-----------------|------------------------|
| 1 | **5V** pin (power header, the side with "POWER") | any hole in the **red `+` rail** near row 1 | red |
| 2 | **GND** pin (either GND right next to 5V) | any hole in the **blue `−` rail** near row 1 | black |

> Use the **5V** pin, **not 3.3V** and **not VIN**.

---

## Step 2: Four 10 kΩ resistors (one per sensor)

10 kΩ color bands: **brown-black-orange-gold** (4-band) or **brown-black-black-red-brown** (5-band).
Resistors have **no direction**; either leg can go either way.

| # | Leg 1 | Leg 2 |
|---|-------|-------|
| 3 | **a5**  | **blue `−` rail**, hole right next to row 5  |
| 4 | **a10** | **blue `−` rail**, hole right next to row 10 |
| 5 | **a15** | **blue `−` rail**, hole right next to row 15 |
| 6 | **a20** | **blue `−` rail**, hole right next to row 20 |

---

## Step 3: Signal jumpers to the Arduino analog pins

The analog pins are on the **"ANALOG IN"** header (labeled A0–A5).

| # | From (breadboard) | To (Arduino) |
|---|-------------------|--------------|
| 7  | **e5**  | **A0** |
| 8  | **e10** | **A1** |
| 9  | **e15** | **A2** |
| 10 | **e20** | **A3** |

---

## Step 4: The four photoresistors

Each LDR has **two long wires** (one per leg). Photoresistors have **no polarity**, so either wire can go in either spot. **Before plugging in, figure out which pair of colors belongs to which quadrant** and write it in the table.

| # | Sensor (looking down at panel face) | Wire 1 goes to | Wire 2 goes to | Wire colors (fill in) |
|---|-------------------------------------|----------------|----------------|-----------------------|
| 11 | **Top-Left**     | **c5**  | **red `+` rail** (near row 5)  | ______ / ______ |
| 12 | **Top-Right**    | **c10** | **red `+` rail** (near row 10) | ______ / ______ |
| 13 | **Bottom-Left**  | **c15** | **red `+` rail** (near row 15) | ______ / ______ |
| 14 | **Bottom-Right** | **c20** | **red `+` rail** (near row 20) | ______ / ______ |

> Stranded wire ends are hard to push into breadboard holes. Twist and tin them with solder, or solder/crimp each one to the end of a male jumper wire.
>
> "Top" and "Bottom" must be on opposite sides of the tilt axis, and "Left" and "Right" on opposite sides of the line across it. See *Sensor placement* in `README.md`.

---

## Step 5: The two servos

Push one **male-to-male jumper** into each of the 3 holes of each servo's plug.

Servo wire colors: **brown = GND**, **red = 5V**, **orange = signal**.
(Some servos use black / red / yellow or white instead.)

**Pan servo** (the one at the base that turns the whole tracker clockwise/counterclockwise):

| # | Servo wire | Goes to |
|---|------------|---------|
| 15 | brown  | **blue `−` rail**, near row 25 |
| 16 | red    | **red `+` rail**, near row 25 |
| 17 | orange | Arduino digital pin **~9** |

**Tilt servo** (the one that tips the panel plate up/down):

| # | Servo wire | Goes to |
|---|------------|---------|
| 18 | brown  | **blue `−` rail**, near row 28 |
| 19 | red    | **red `+` rail**, near row 28 |
| 20 | orange | Arduino digital pin **~10** |

> The digital pins are on the header labeled **"DIGITAL (PWM~)"**. Pins 9 and 10 have a `~` next to them.

---

## Step 6 (recommended): 100 µF capacitor for steady servo power

This stops the Arduino from resetting or the servos from twitching when both move at once.

| # | Capacitor leg | Goes to |
|---|---------------|---------|
| 21 | **long leg (+)** | **red `+` rail**, near row 27 |
| 22 | **short leg (−)**, the side with the stripe of minus signs | **blue `−` rail**, right next to it |

> ⚠️ This capacitor **has a direction**. Backwards, it can get hot or pop. Long leg goes to **red +**, stripe side goes to **blue −**.

---

## Step 7: Check before plugging in USB

- [ ] Arduino **5V → red `+` rail** and **GND → blue `−` rail** (not swapped!)
- [ ] Rows **5, 10, 15, 20** each have exactly three things: a resistor in **a**, an LDR wire in **c**, a jumper in **e**
- [ ] The other leg of each **resistor** is in the **blue `−`** rail
- [ ] The other wire of each **LDR** is in the **red `+`** rail
- [ ] Nothing is plugged into columns **f–j** (the other side of the groove)
- [ ] Row 5 → **A0**, row 10 → **A1**, row 15 → **A2**, row 20 → **A3**
- [ ] Pan servo signal → **pin 9**, tilt servo signal → **pin 10**
- [ ] Both servos' **brown → `−`**, **red → `+`**
- [ ] Capacitor: long leg on **red +**, stripe on **blue −**

Then plug in USB and do the **Sensor test** in `README.md` (set `SENSOR_TEST_MODE = true`). Covering each sensor should make only its own number drop.

---

## Common mistakes

| Symptom | Likely wiring mistake |
|---------|-----------------------|
| A sensor always reads ~1023 | Its 10 kΩ resistor isn't reaching the `−` rail |
| A sensor always reads ~0 | Its LDR's second wire isn't in the `+` rail, or the LDR wire is in the wrong row |
| A sensor reads random, jumpy numbers | The jumper to the analog pin is loose or in the wrong row |
| Covering one sensor changes a different number | LDR wires are in the wrong rows; swap them to match the table |
| Servo doesn't move at all | Signal wire on the wrong pin, or brown/red not reaching the rails |
| Arduino resets / USB disconnects when servos move | Add the 100 µF capacitor; if it still happens, power servos from a separate 5V supply with its GND tied to the `−` rail |
| Readings go **down** with more light | LDR and resistor positions swapped (LDR should go to `+`, resistor to `−`) |
