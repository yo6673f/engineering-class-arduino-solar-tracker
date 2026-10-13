/*
 * ============================================================================
 *  Dual-Axis Arduino Solar Tracker
 *  ENGR 102B - University of Arizona
 *  Board: Arduino Uno R3
 * ============================================================================
 *
 *  HOW IT WORKS
 *  ------------
 *  Four photoresistors (LDRs) sit in the four quadrants of a "+" shaped wall
 *  above the solar panel. When the light is NOT straight on, the wall casts a
 *  shadow on the sensors that face away from it, so those read darker.
 *
 *      Looking straight down at the panel face:
 *
 *                  TOP
 *           +------+------+
 *           |  TL  |  TR  |
 *     LEFT  +------+------+  RIGHT
 *           |  BL  |  BR  |
 *           +------+------+
 *                 BOTTOM
 *
 *  - Top pair vs. bottom pair  -> drives the TILT servo (up / down)
 *  - Left pair vs. right pair  -> drives the PAN  servo (CW / CCW)
 *
 *  The solar panel's voltage (on A4) is shown in the status line too.
 *
 *  ON STARTUP it:
 *    1. moves both servos smoothly to the middle (HOME),
 *    2. SCANS around to find the brightest direction and points there,
 *    3. uses what it saw during the scan to CHECK which way each servo has
 *       to turn to go toward the light (and fixes it if it's backwards),
 *    4. then tracks. If it ever loses the light, it scans again.
 *
 *  SERIAL COMMANDS (Serial Monitor, 9600 baud - type, then press Enter)
 *    h      = go HOME (both servos to the middle) and pause tracking
 *    p90    = move PAN to 90 degrees and pause (any number works)
 *    t45    = move TILT to 45 degrees and pause
 *    g      = GO: scan for the light, check directions, start tracking again
 *    c      = calibrate the sensors (white paper over the + wall, light on top)
 *    r      = reset sensor calibration
 *
 *  IMPORTANT: This file and arduino/SolarTracker/SolarTracker.ino are meant to
 *  be IDENTICAL. If you change one, copy the change into the other.
 *
 *  See README.md for wiring, first-run steps, calibration, and tuning.
 * ============================================================================
 */

#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

// ============================================================================
//  PIN ASSIGNMENTS
// ============================================================================

// Photoresistors (each one is half of a voltage divider with a 10k resistor)
const uint8_t LDR_TOP_LEFT_PIN     = A0;
const uint8_t LDR_TOP_RIGHT_PIN    = A1;
const uint8_t LDR_BOTTOM_LEFT_PIN  = A2;
const uint8_t LDR_BOTTOM_RIGHT_PIN = A3;

// Servos
const uint8_t PAN_SERVO_PIN  = 9;   // turns the tracker clockwise / counterclockwise
const uint8_t TILT_SERVO_PIN = 10;  // tilts the panel plate up / down

// ============================================================================
//  SOLAR PANEL MONITOR
// ============================================================================
//  The panel feeds A4 through a divide-by-3 voltage divider (three 10k
//  resistors, see WIRING.md), so A4 never sees more than 5V. The status line
//  then shows   panel:5.73V (max 6.10)   and, with a load resistor, watts.

const bool    PANEL_MONITOR = true;
const uint8_t PANEL_PIN     = A4;
const float   PANEL_DIVIDER = 3.0;   // (top resistors + bottom) / bottom = 30k / 10k

// The Arduino's real 5V. Over USB it's often 4.8 - 5.0. For better accuracy,
// measure between the 5V and GND pins with a multimeter and put it here.
const float   ARDUINO_VOLTS = 5.0;

// Load resistor across the panel (for measuring power), in ohms.
// 0 = no load resistor, so only voltage is shown. Set to 220 (or whatever
// you used) after adding one, and the status line also shows watts (V*V/R).
const float   PANEL_LOAD_OHMS = 0;

// ============================================================================
//  SERVO LIMITS  (degrees, 0 - 180)   <-- SET THESE FOR YOUR BUILD
// ============================================================================
//  If a servo is told to go further than the 3D printed parts allow, it
//  stalls against the part: it buzzes, jitters, gets hot, and the tracker
//  thinks it's somewhere it isn't. Find your real limits like this:
//    1. Upload, open the Serial Monitor, type  h  + Enter (goes to the middle).
//    2. Type  t40  + Enter, then  t30, t20 ... until the plate stops moving
//       or hits something. Add ~5 degrees back and that's TILT_MIN.
//    3. Do the same going up (t140, t150, ...) for TILT_MAX, and with
//       p for the pan servo (PAN_MIN / PAN_MAX).

const int PAN_MIN  = 10;
const int PAN_MAX  = 170;
const int PAN_HOME = 90;

const int TILT_MIN  = 30;
const int TILT_MAX  = 150;
const int TILT_HOME = 90;

// Tilt servo angle at which the panel faces STRAIGHT UP at the sky (flat).
// Check it: type  t90  + Enter. If the plate isn't flat, try other numbers
// until it is, and put that number here.
//
// Why it matters: when the plate tilts past flat to the other side, the panel
// is "upside down" relative to the pan servo, so the pan direction has to
// flip to keep turning toward the light. The code does that at this angle.
const int  TILT_FLAT_ANGLE    = 90;
const bool FLIP_PAN_PAST_FLAT = true;

// The plate has to go this many degrees PAST flat before the pan direction
// flips. Stops the pan from flip-flopping when the light is nearly overhead.
const float FLAT_HYSTERESIS = 6.0;

// ============================================================================
//  DIRECTION SETTINGS
// ============================================================================
//  With AUTO_DIRECTION_CHECK on (recommended), the tracker figures these out
//  by itself during every scan, so you normally never touch them. They're
//  only the starting guess (and what's used if the scan can't tell).

const bool AUTO_DIRECTION_CHECK = true;
const bool TILT_REVERSED = false;
const bool PAN_REVERSED  = false;

// Set to true ONLY if your readings go DOWN when you shine light on a sensor.
// (That happens if the LDR and 10k resistor are swapped in the divider.)
const bool INVERT_LDR_READINGS = false;

// ============================================================================
//  SCANNING  (finding the light on startup, or when it's been lost)
// ============================================================================

const bool  SCAN_ON_STARTUP = true;
const int   SCAN_ROWS = 4;              // tilt levels, as offsets from flat:
const float SCAN_TILT_OFFSETS[SCAN_ROWS] = {-60, -30, 30, 60};
const int   SCAN_COLS = 5;              // pan positions per tilt level
const float SCAN_SPEED = 0.12;          // degrees per millisecond while scanning
                                        // (slower = gentler on USB power)
const unsigned long SCAN_SETTLE_MS = 200;  // wait for wobble before measuring

// The direction check has to be at least this sure (0 - 1) before it changes
// a direction. Below that it keeps the current setting.
const float DIRECTION_MIN_CONFIDENCE = 0.3;
// ...and the brightest scan position must be at least this many times
// brighter than the darkest one (otherwise the light is too even to tell).
const float DIRECTION_MIN_CONTRAST = 1.5;

// "Lost the light": the average raw reading has stayed below this fraction
// of what it was right after the last scan, for LOST_TIME_MS. Then it scans
// again (but not more often than RESCAN_COOLDOWN_MS).
const float LOST_FRACTION      = 0.50;
const unsigned long LOST_TIME_MS       = 5000;
const unsigned long RESCAN_COOLDOWN_MS = 20000;

// ============================================================================
//  PAN "WRAP-AROUND"  (reaching light that's behind the pan end stops)
// ============================================================================
//  The pan servo can't turn all the way around. If the light is further
//  around than that, the panel can still point at it by swinging the pan to
//  its OTHER end and flipping the tilt to the other side of flat. This only
//  happens when the pan is at an end stop, the light is CLEARLY further that
//  way (error >= WRAP_MIN_ERROR) for WRAP_CONFIRM_MS, and it can't happen
//  again until the pan has moved WRAP_REARM_DEGREES on its own, so it can
//  never bounce back and forth. Set to false to just wait at the end stop.

const bool  WRAP_PAN_AT_LIMITS   = true;
const float WRAP_MIN_ERROR       = 0.30;   // 30% left/right difference
const unsigned long WRAP_CONFIRM_MS = 1500;
const float WRAP_REARM_DEGREES   = 20.0;
const unsigned long WRAP_SWING_MS   = 1500; // how long the big swing takes

// ============================================================================
//  TUNING
// ============================================================================
//  The "error" for each axis is a ratio from -1 to +1:
//  0 = both sides equally lit, 0.5 = one side much brighter.
//  Watch the  up/down  and  left/right  percentages in the Serial Monitor
//  to see what your tracker is seeing.

// Start moving an axis when its error gets bigger than START_THRESHOLD, then
// keep moving until it's smaller than STOP_THRESHOLD. Having two thresholds
// stops the tracker from twitching on and off around a single cutoff.
const float START_THRESHOLD = 0.05;
const float STOP_THRESHOLD  = 0.02;

// How hard to push toward the light: degrees moved per update for each unit
// of error. With GAIN = 12, a 10% error asks for a 1.2 degree step.
// Too slow / barely moves -> raise it. Wobbles or twitches -> lower it.
const float GAIN = 12.0;

// Never move more than this many degrees in one update (keeps motion smooth
// and gives the servos and plate time to keep up).
const float MAX_STEP = 2.0;

// Sensor smoothing, 0 - 1. Smaller = smoother but slower to react.
const float SMOOTHING = 0.3;

// If the average raw reading (0 - 1023) drops below this, it's too dark to
// track (night / covered), so hold position instead of wandering.
const int DARK_THRESHOLD = 30;

// Number of analogRead() samples averaged per sensor (reduces noise).
const int SAMPLES_PER_READ = 8;

// Time between control updates, in milliseconds.
const unsigned long LOOP_INTERVAL_MS = 40;

// ============================================================================
//  DEBUG / TEST OPTIONS
// ============================================================================

// Print sensor readings and servo angles to the Serial Monitor (9600 baud).
const bool SERIAL_DEBUG = true;
const unsigned long DEBUG_PRINT_INTERVAL_MS = 250;

// SENSOR TEST MODE: servos hold at HOME and never move. The Serial Monitor
// shows each sensor's reading and which direction the light is coming from.
// Use it to confirm which wire color goes to which quadrant.
const bool SENSOR_TEST_MODE = false;

// SERVO CENTER MODE: on power-up both servos go to exactly 90 degrees and
// STAY there (no scan, no tracking). Use it to re-attach the servo arms so
// that 90 degrees really is the middle of your mechanism (see README).
// You can still type p / t commands to test, and g to start tracking.
const bool SERVO_CENTER_MODE = false;

// ============================================================================
//  CALIBRATION SETTINGS
// ============================================================================

const unsigned long COUNTDOWN_MS   = 5000;  // time to get the light set up
const unsigned long CAL_MEASURE_MS = 2000;  // time spent measuring
const float CAL_MIN_GAIN = 0.1;  // corrections are allowed from 0.1x to 10x
const float CAL_MAX_GAIN = 10.0;
const float CAL_WARN_GAIN = 3.0; // bigger than this (or below 1/this): print a warning
const int   CAL_DEAD_RAW  = 20;    // a sensor reading below this is dead/unplugged
const int   CAL_MAXED_RAW = 1005;  // above this a sensor is maxed out (light too bright)
const int   CAL_EEPROM_ADDR = 0;
const uint16_t CAL_MAGIC = 0x5A19;  // marks the EEPROM data as valid

// ============================================================================
//  GLOBALS
// ============================================================================

Servo panServo;
Servo tiltServo;

// Positions are kept as floats so small steps add up smoothly.
float panPos  = PAN_HOME;
float tiltPos = TILT_HOME;

// Directions actually in use (start from the settings above, then updated
// by the automatic direction check).
bool tiltReversed = TILT_REVERSED;
bool panReversed  = PAN_REVERSED;

// Sensor order everywhere: 0 = TL, 1 = TR, 2 = BL, 3 = BR
const uint8_t LDR_PINS[4] = {
  LDR_TOP_LEFT_PIN, LDR_TOP_RIGHT_PIN, LDR_BOTTOM_LEFT_PIN, LDR_BOTTOM_RIGHT_PIN
};

// One scan measurement: the two errors and the total brightness.
struct ScanSample {
  float vert;
  float horiz;
  float total;
};

// Calibration: each sensor's brightness is multiplied by its gain so all four
// read the same under the same light. Saved in EEPROM, so it survives
// power-off and re-uploading the code.
struct Calibration {
  uint16_t magic;
  float gain[4];
};
Calibration cal;

float raw[4];        // latest raw readings, 0 - 1023 (what you saw in test mode)
float smoothed[4];   // smoothed, calibrated brightness used for tracking
float smoothedRawAvg = 0;  // smoothed average raw reading (for the dark check)
float panelVolts = 0;      // smoothed solar panel voltage
float panelMaxVolts = 0;   // highest panel voltage seen since startup

bool manualMode = false;       // true after h / p / t: tracking paused
bool tiltMoving = false;       // start/stop threshold state for each axis
bool panMoving  = false;
bool tiltOnHighSide = true;    // is the plate past flat toward the high angles?

bool wrapArmed = true;
bool wrapPending = false;
unsigned long wrapPendingSince = 0;
float wrapLandingPos = 0;

float scanBestRaw = 0;         // average raw reading right after the last scan
bool lostPending = false;
unsigned long lostSince = 0;
unsigned long lastScanEnd = 0;

char cmdBuffer[10];            // Serial command being typed
uint8_t cmdLength = 0;
unsigned long lastCmdCharMs = 0;

unsigned long lastDebugPrint = 0;

// Function prototypes (needed by PlatformIO; harmless in the Arduino IDE)
float readLdrRaw(uint8_t pin);
float brightnessFromRaw(float rawValue);
float readPanelVolts();
void  readAllSensors(bool resetSmoothing);
void  computeErrors(const float *b, float &vertError, float &horizError);
void  measureAveraged(float &vertError, float &horizError, float &total);
float clampFloat(float value, float low, float high);
float computeStep(float error, bool &moving);
void  writeServos();
void  slewServosTo(float newPan, float newTilt, unsigned long durationMs,
                   unsigned long settleMs = 300);
void  moveAtScanSpeed(float newPan, float newTilt);
void  resetTrackingState();
void  scanForLight();
void  measureAt(float pan, float tilt, ScanSample &sample,
                float &bestB, float &bestPan, float &bestTilt);
void  decideDirections(ScanSample grid[SCAN_ROWS][SCAN_COLS], const float *rowTilt);
void  printDirection(const __FlashStringHelper *axis, bool reversed, bool setting,
                     float vote, float size);
void  countdown();
void  handleSerialCommands();
void  runCommand();
void  loadCalibration();
void  resetCalibration();
void  runCalibration();
void  printCalibration();
void  printDebug(float vertError, float horizError, const char *status);

// ============================================================================
//  SETUP
// ============================================================================

void setup() {
  Serial.begin(9600);

  // ---- Full reset: everything back to the starting state ----
  panPos  = PAN_HOME;
  tiltPos = TILT_HOME;
  tiltReversed = TILT_REVERSED;
  panReversed  = PAN_REVERSED;
  manualMode = false;
  scanBestRaw = 0;
  cmdLength = 0;
  resetTrackingState();

  panServo.attach(PAN_SERVO_PIN);
  tiltServo.attach(TILT_SERVO_PIN);
  writeServos();

  Serial.println(F("Solar tracker starting..."));
  Serial.println(F("(If you see this line again while it's running, the Arduino"));
  Serial.println(F(" reset - usually a servo power problem. See README.)"));
  Serial.println(F("HOME: both servos to the middle."));

  loadCalibration();
  printCalibration();
  Serial.println(F("Commands: h=home  p90/t45=move a servo  g=scan+track  c=calibrate"));

  if (SENSOR_TEST_MODE) {
    Serial.println(F("*** SENSOR TEST MODE: servos will NOT move ***"));
    Serial.println(F("Shine a light on each sensor - its number should go UP."));
  }

  delay(1500);  // give the servos time to reach the middle
  readAllSensors(true);

  if (SERVO_CENTER_MODE) {
    panPos = 90;
    tiltPos = 90;
    writeServos();
    manualMode = true;
    Serial.println(F("*** SERVO CENTER MODE: both servos held at exactly 90 degrees ***"));
    Serial.println(F("Re-attach the servo arms so the plate is centered and flat."));
    Serial.println(F("Then set SERVO_CENTER_MODE = false and re-upload."));
  } else if (SCAN_ON_STARTUP && !SENSOR_TEST_MODE) {
    scanForLight();
  }
}

// ============================================================================
//  MAIN LOOP
// ============================================================================

void loop() {
  handleSerialCommands();

  // ---- 1. Read all four sensors ----
  readAllSensors(false);

  // ---- 2. Errors, each between -1 and +1 ----
  //   vertError  > 0  -> light is coming from the TOP
  //   horizError > 0  -> light is coming from the LEFT
  float vertError, horizError;
  computeErrors(smoothed, vertError, horizError);

  // ---- 3. Decide whether to move ----
  if (SENSOR_TEST_MODE) {
    printDebug(vertError, horizError, "TEST");
    delay(LOOP_INTERVAL_MS);
    return;
  }

  if (manualMode) {
    printDebug(vertError, horizError, "MANUAL");
    delay(LOOP_INTERVAL_MS);
    return;
  }

  if (smoothedRawAvg < DARK_THRESHOLD) {
    tiltMoving = panMoving = false;
    wrapPending = false;
    lostPending = false;
    printDebug(vertError, horizError, "DARK-HOLD");
    delay(LOOP_INTERVAL_MS);
    return;
  }

  // ---- 4. Lost the light? (much darker than right after the scan) ----
  // Uses the plain 0 - 1023 readings, which stay steady even when the
  // sensors are bright (the brightness conversion gets jumpy near 1023).
  if (scanBestRaw > 0 && smoothedRawAvg < LOST_FRACTION * scanBestRaw) {
    if (!lostPending) {
      lostPending = true;
      lostSince = millis();
    } else if (millis() - lostSince >= LOST_TIME_MS &&
               millis() - lastScanEnd >= RESCAN_COOLDOWN_MS) {
      Serial.print(F("LOST the light (average reading "));
      Serial.print((int)smoothedRawAvg);
      Serial.print(F(", was "));
      Serial.print((int)scanBestRaw);
      Serial.println(F(" after the scan) - scanning for it again..."));
      scanForLight();
      return;
    }
  } else {
    lostPending = false;
  }

  // ---- 5. TILT: tip the panel toward the brighter (top or bottom) side ----
  // A bigger tilt angle tips the panel face toward the TOP sensors, unless
  // tiltReversed.
  float tiltDir = tiltReversed ? -1.0 : 1.0;
  tiltPos += tiltDir * computeStep(vertError, tiltMoving);
  tiltPos = clampFloat(tiltPos, TILT_MIN, TILT_MAX);

  // Which side of flat is the plate on? It has to go FLAT_HYSTERESIS degrees
  // past flat before this changes, so the pan doesn't flip-flop near flat.
  if (tiltPos > TILT_FLAT_ANGLE + FLAT_HYSTERESIS) tiltOnHighSide = true;
  if (tiltPos < TILT_FLAT_ANGLE - FLAT_HYSTERESIS) tiltOnHighSide = false;

  // ---- 6. PAN: turn the panel toward the brighter (left or right) side ----
  // While the plate is on the high-angle side of flat, a bigger pan angle
  // swings the panel face toward the LEFT sensors, unless panReversed. On the
  // other side of flat the pan's effect is mirrored, so it's flipped again.
  float panDir = panReversed ? -1.0 : 1.0;
  if (FLIP_PAN_PAST_FLAT && !tiltOnHighSide) {
    panDir = -panDir;
  }
  float panStep = panDir * computeStep(horizError, panMoving);

  // ---- 7. Pan is at an end stop but the light is clearly further around ----
  if (!wrapArmed && fabs(panPos - wrapLandingPos) >= WRAP_REARM_DEGREES) {
    wrapArmed = true;
  }
  bool pushingPastMax = panPos >= PAN_MAX && panStep > 0;
  bool pushingPastMin = panPos <= PAN_MIN && panStep < 0;
  bool wantWrap = WRAP_PAN_AT_LIMITS && wrapArmed &&
                  (pushingPastMax || pushingPastMin) &&
                  fabs(horizError) >= WRAP_MIN_ERROR;

  if (!wantWrap) {
    wrapPending = false;
  } else if (!wrapPending) {
    wrapPending = true;               // start the confirmation timer
    wrapPendingSince = millis();
  } else if (millis() - wrapPendingSince >= WRAP_CONFIRM_MS) {
    // Swing pan 180 degrees to the other end and mirror the tilt across flat.
    // This points the panel the same way, but now it can keep following.
    float newPan  = clampFloat(pushingPastMax ? panPos - 180 : panPos + 180, PAN_MIN, PAN_MAX);
    float newTilt = clampFloat(2 * TILT_FLAT_ANGLE - tiltPos, TILT_MIN, TILT_MAX);
    lastDebugPrint = 0;
    printDebug(vertError, horizError, "WRAP");
    slewServosTo(newPan, newTilt, WRAP_SWING_MS);

    tiltOnHighSide = !tiltOnHighSide;  // the plate is now on the other side
    wrapArmed = false;                 // no second wrap until pan moves away
    wrapLandingPos = panPos;
    wrapPending = false;
    tiltMoving = panMoving = false;
    readAllSensors(true);              // fresh readings from the new position
    return;
  }

  panPos = clampFloat(panPos + panStep, PAN_MIN, PAN_MAX);

  // ---- 8. Send the new positions to the servos ----
  writeServos();

  printDebug(vertError, horizError, (tiltMoving || panMoving) ? "TRACK" : "HOLD");
  delay(LOOP_INTERVAL_MS);
}

// ============================================================================
//  SENSOR HELPERS
// ============================================================================

// Reads one photoresistor, averaged over several samples.
// Returns 0 - 1023 where a BIGGER number always means MORE light.
float readLdrRaw(uint8_t pin) {
  analogRead(pin);  // throw away first read after switching pins (more accurate)

  long total = 0;
  for (int i = 0; i < SAMPLES_PER_READ; i++) {
    total += analogRead(pin);
  }
  float value = (float)total / SAMPLES_PER_READ;

  if (INVERT_LDR_READINGS) {
    value = 1023 - value;
  }
  return value;
}

// Converts a raw reading into "brightness" (proportional to how well the
// photoresistor conducts).
//
// Why: with the 10k divider, bright light squashes every reading up near
// 1023, so a lit sensor (990) and a shaded one (850) look only ~8% apart and
// the tracker can't tell it's off target. Undoing the divider math spreads
// them back out (here about 5x apart), so the tracker sees shadows clearly.
// It also makes calibration correct at any brightness.
//
// Readings are capped at 1015: above that the sensor is basically maxed out
// and the conversion would just blow up the noise.
float brightnessFromRaw(float rawValue) {
  if (rawValue > 1015) rawValue = 1015;
  if (rawValue < 0)    rawValue = 0;
  return rawValue / (1023.5 - rawValue);
}

// Reads all four sensors, applies calibration, and smooths them over time.
// resetSmoothing = true throws away the old history (used after big moves).
void readAllSensors(bool resetSmoothing) {
  float rawAvg = 0;
  for (int i = 0; i < 4; i++) {
    raw[i] = readLdrRaw(LDR_PINS[i]);
    rawAvg += raw[i] / 4.0;
    float value = brightnessFromRaw(raw[i]) * cal.gain[i];
    if (resetSmoothing) {
      smoothed[i] = value;
    } else {
      smoothed[i] += SMOOTHING * (value - smoothed[i]);
    }
  }
  // Smoothing the dark check too means a single glitchy reading can't
  // trigger DARK-HOLD.
  if (resetSmoothing) {
    smoothedRawAvg = rawAvg;
  } else {
    smoothedRawAvg += SMOOTHING * (rawAvg - smoothedRawAvg);
  }

  if (PANEL_MONITOR) {
    float v = readPanelVolts();
    panelVolts = resetSmoothing ? v : panelVolts + SMOOTHING * (v - panelVolts);
    if (panelVolts > panelMaxVolts) panelMaxVolts = panelVolts;
  }
}

// Reads the solar panel's voltage (through the voltage divider on A4).
float readPanelVolts() {
  analogRead(PANEL_PIN);  // throw away first read after switching pins
  long total = 0;
  for (int i = 0; i < SAMPLES_PER_READ; i++) {
    total += analogRead(PANEL_PIN);
  }
  float pinVolts = ((float)total / SAMPLES_PER_READ) / 1023.0 * ARDUINO_VOLTS;
  return pinVolts * PANEL_DIVIDER;
}

// Turns four brightness values (TL, TR, BL, BR) into the two errors.
// Dividing by the total makes the tracker behave the same in dim light and
// bright light.
void computeErrors(const float *b, float &vertError, float &horizError) {
  float top    = b[0] + b[1];
  float bottom = b[2] + b[3];
  float left   = b[0] + b[2];
  float right  = b[1] + b[3];
  vertError  = (top + bottom > 0) ? (top - bottom) / (top + bottom) : 0;
  horizError = (left + right > 0) ? (left - right) / (left + right) : 0;
}

// Averages the sensors for a moment and returns the errors and the total
// brightness (used by scanning and the direction check, where accuracy
// matters more than speed).
void measureAveraged(float &vertError, float &horizError, float &total) {
  float sum[4] = {0, 0, 0, 0};
  const int count = 6;
  for (int n = 0; n < count; n++) {
    for (int i = 0; i < 4; i++) {
      sum[i] += brightnessFromRaw(readLdrRaw(LDR_PINS[i])) * cal.gain[i];
    }
    delay(10);
  }
  computeErrors(sum, vertError, horizError);
  total = (sum[0] + sum[1] + sum[2] + sum[3]) / count;
}

// ============================================================================
//  MOTION HELPERS
// ============================================================================

float clampFloat(float value, float low, float high) {
  if (value < low)  return low;
  if (value > high) return high;
  return value;
}

// Turns an error (-1 to +1) into a step size in degrees, with two thresholds:
// an axis starts moving above START_THRESHOLD and keeps going until the
// error drops below STOP_THRESHOLD. Returns 0 while the axis is resting.
float computeStep(float error, bool &moving) {
  float size = fabs(error);
  if (!moving && size > START_THRESHOLD) moving = true;
  if (moving && size < STOP_THRESHOLD)   moving = false;
  if (!moving) return 0;
  return clampFloat(error * GAIN, -MAX_STEP, MAX_STEP);
}

// Sends panPos / tiltPos to the servos. Uses microseconds instead of whole
// degrees so small steps come out smooth instead of in 1-degree jumps.
void writeServos() {
  const float usPerDegree = (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180.0;
  panServo.writeMicroseconds((int)(MIN_PULSE_WIDTH + panPos * usPerDegree + 0.5));
  tiltServo.writeMicroseconds((int)(MIN_PULSE_WIDTH + tiltPos * usPerDegree + 0.5));
}

// Moves both servos smoothly to a new position over durationMs, instead of
// slamming them there (big sudden moves can brown out the Arduino).
void slewServosTo(float newPan, float newTilt, unsigned long durationMs,
                  unsigned long settleMs) {
  const unsigned long stepMs = 20;
  int steps = durationMs / stepMs;
  if (steps < 1) steps = 1;
  float startPan = panPos, startTilt = tiltPos;
  for (int i = 1; i <= steps; i++) {
    float t = (float)i / steps;
    panPos  = startPan  + (newPan  - startPan)  * t;
    tiltPos = startTilt + (newTilt - startTilt) * t;
    writeServos();
    delay(stepMs);
  }
  delay(settleMs);  // let the plate stop wobbling
}

// Smooth move whose duration depends on how far it has to go.
void moveAtScanSpeed(float newPan, float newTilt) {
  newPan  = clampFloat(newPan,  PAN_MIN,  PAN_MAX);
  newTilt = clampFloat(newTilt, TILT_MIN, TILT_MAX);
  float distance = fabs(newPan - panPos);
  if (fabs(newTilt - tiltPos) > distance) distance = fabs(newTilt - tiltPos);
  unsigned long duration = (unsigned long)(distance / SCAN_SPEED);
  if (duration < 100) duration = 100;
  slewServosTo(newPan, newTilt, duration, SCAN_SETTLE_MS);
}

// Clears everything the tracking logic remembers between updates.
void resetTrackingState() {
  tiltMoving = panMoving = false;
  tiltOnHighSide = (tiltPos >= TILT_FLAT_ANGLE);
  wrapArmed = true;
  wrapPending = false;
  lostPending = false;
}

// ============================================================================
//  SCAN FOR THE LIGHT
// ============================================================================
//  Visits a grid of positions (4 tilt levels x 5 pan positions, plus flat),
//  measures the total brightness at each one, then zooms in around the
//  brightest spot. Afterwards the panel points roughly at the light, and
//  normal tracking does the fine aiming. The grid measurements are also used
//  to check the servo directions (see decideDirections).

void scanForLight() {
  Serial.println();
  Serial.println(F("SCAN: looking for the light - hold the light still..."));

  ScanSample grid[SCAN_ROWS][SCAN_COLS];
  float rowTilt[SCAN_ROWS], colPan[SCAN_COLS];
  for (int r = 0; r < SCAN_ROWS; r++) {
    rowTilt[r] = clampFloat(TILT_FLAT_ANGLE + SCAN_TILT_OFFSETS[r], TILT_MIN, TILT_MAX);
  }
  for (int c = 0; c < SCAN_COLS; c++) {
    colPan[c] = PAN_MIN + (float)(PAN_MAX - PAN_MIN) * c / (SCAN_COLS - 1);
  }

  float bestB = -1, bestPan = PAN_HOME, bestTilt = TILT_FLAT_ANGLE;
  ScanSample sample;

  // Coarse pass: flat first, then each tilt level, snaking back and forth.
  measureAt(PAN_HOME, TILT_FLAT_ANGLE, sample, bestB, bestPan, bestTilt);
  const int rowOrder[SCAN_ROWS] = {2, 3, 1, 0};
  for (int k = 0; k < SCAN_ROWS; k++) {
    int r = rowOrder[k];
    for (int j = 0; j < SCAN_COLS; j++) {
      int c = (k % 2 == 0) ? j : SCAN_COLS - 1 - j;
      measureAt(colPan[c], rowTilt[r], grid[r][c], bestB, bestPan, bestTilt);
    }
  }

  if (AUTO_DIRECTION_CHECK) {
    decideDirections(grid, rowTilt);
  }

  // Fine passes: check around the best spot, closer each time.
  const float zoom[2] = {15, 8};
  for (int z = 0; z < 2; z++) {
    float cp = bestPan, ct = bestTilt;
    measureAt(cp + zoom[z], ct, sample, bestB, bestPan, bestTilt);
    measureAt(cp - zoom[z], ct, sample, bestB, bestPan, bestTilt);
    measureAt(cp, ct + zoom[z], sample, bestB, bestPan, bestTilt);
    measureAt(cp, ct - zoom[z], sample, bestB, bestPan, bestTilt);
  }

  moveAtScanSpeed(bestPan, bestTilt);
  Serial.print(F("SCAN: brightest at pan "));
  Serial.print((int)(bestPan + 0.5));
  Serial.print(F(", tilt "));
  Serial.println((int)(bestTilt + 0.5));

  readAllSensors(true);
  scanBestRaw = smoothedRawAvg;
  resetTrackingState();
  lastScanEnd = millis();
  Serial.println(F("Tracking..."));
  Serial.println();
}

// Moves to one position, measures it, and remembers it if it's the
// brightest so far.
void measureAt(float pan, float tilt, ScanSample &sample,
               float &bestB, float &bestPan, float &bestTilt) {
  pan  = clampFloat(pan,  PAN_MIN,  PAN_MAX);
  tilt = clampFloat(tilt, TILT_MIN, TILT_MAX);
  moveAtScanSpeed(pan, tilt);
  measureAveraged(sample.vert, sample.horiz, sample.total);
  if (sample.total > bestB) {
    bestB = sample.total;
    bestPan = pan;
    bestTilt = tilt;
  }
}

// ============================================================================
//  DIRECTION CHECK  (uses the scan grid)
// ============================================================================
//  Idea: at each scan position the sensors say which side the light is on.
//  If the directions are set right, stepping that way in the grid should
//  land on a BRIGHTER position. So for every pair of neighboring positions
//  we check: did the side the sensors pointed to turn out brighter? Every
//  pair "votes" (bright, clear pairs count more), and the total decides.

void decideDirections(ScanSample grid[SCAN_ROWS][SCAN_COLS], const float *rowTilt) {
  Serial.println(F("CHECK: which way is toward the light?"));

  float darkest = grid[0][0].total, brightest = grid[0][0].total;
  for (int r = 0; r < SCAN_ROWS; r++) {
    for (int c = 0; c < SCAN_COLS; c++) {
      if (grid[r][c].total < darkest)   darkest = grid[r][c].total;
      if (grid[r][c].total > brightest) brightest = grid[r][c].total;
    }
  }
  if (brightest < DIRECTION_MIN_CONTRAST * darkest) {
    Serial.println(F("  light is too even to tell - keeping the current directions"));
    return;
  }

  // --- Tilt: neighbors in the same column, B has the bigger tilt angle ---
  // Normal tilt: a bigger angle tips the face toward the TOP sensors, so
  // where the light is from the top (vert > 0), B should be brighter.
  float tiltVote = 0, tiltSize = 0;
  for (int c = 0; c < SCAN_COLS; c++) {
    for (int r = 0; r + 1 < SCAN_ROWS; r++) {
      if (rowTilt[r + 1] - rowTilt[r] < 1) continue;  // rows squashed by limits
      ScanSample &A = grid[r][c], &B = grid[r + 1][c];
      float vote = (B.total - A.total) * (A.vert + B.vert);
      tiltVote += vote;
      tiltSize += fabs(vote);
    }
  }

  // --- Pan: neighbors in the same row, B has the bigger pan angle ---
  // Normal pan, high side of flat: a bigger angle swings the face toward the
  // LEFT sensors, so where the light is from the left (horiz > 0), B should
  // be brighter. Mirrored on the other side of flat.
  float panVote = 0, panSize = 0;
  for (int r = 0; r < SCAN_ROWS; r++) {
    if (fabs(rowTilt[r] - TILT_FLAT_ANGLE) < 10) continue;  // pan does little near flat
    float side = (rowTilt[r] > TILT_FLAT_ANGLE) ? 1.0 : -1.0;
    for (int c = 0; c + 1 < SCAN_COLS; c++) {
      ScanSample &A = grid[r][c], &B = grid[r][c + 1];
      float vote = side * (B.total - A.total) * (A.horiz + B.horiz);
      panVote += vote;
      panSize += fabs(vote);
    }
  }

  if (tiltSize > 0 && fabs(tiltVote) / tiltSize >= DIRECTION_MIN_CONFIDENCE) {
    tiltReversed = (tiltVote < 0);
  }
  printDirection(F("TILT"), tiltReversed, TILT_REVERSED, tiltVote, tiltSize);

  if (panSize > 0 && fabs(panVote) / panSize >= DIRECTION_MIN_CONFIDENCE) {
    panReversed = (panVote < 0);
  }
  printDirection(F("PAN "), panReversed, PAN_REVERSED, panVote, panSize);
}

void printDirection(const __FlashStringHelper *axis, bool reversed, bool setting,
                    float vote, float size) {
  int confidence = (size > 0) ? (int)(100 * fabs(vote) / size) : 0;
  Serial.print(F("  "));
  Serial.print(axis);
  Serial.print(F(": "));
  if (confidence < DIRECTION_MIN_CONFIDENCE * 100) {
    Serial.print(F("can't tell (light too dim or too even), keeping "));
    Serial.println(reversed ? F("reversed") : F("normal"));
    return;
  }
  Serial.print(reversed ? F("reversed") : F("normal"));
  Serial.print(F("  (sure: "));
  Serial.print(confidence);
  Serial.print(F("%)"));
  Serial.println(reversed == setting ? F("") : F("  <- auto-corrected"));
}

void countdown() {
  for (unsigned long left = COUNTDOWN_MS / 1000; left > 0; left--) {
    Serial.print(F("Starting in "));
    Serial.print(left);
    Serial.println(F("..."));
    delay(1000);
  }
}

// ============================================================================
//  SERIAL COMMANDS
// ============================================================================
//  Collects typed characters into a command and runs it on Enter (or after a
//  short pause, in case the Serial Monitor isn't sending a line ending).

void handleSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (cmdLength > 0) runCommand();
      continue;
    }
    if (cmdLength < (uint8_t)(sizeof(cmdBuffer) - 1)) {
      cmdBuffer[cmdLength++] = c;
    }
    lastCmdCharMs = millis();
  }
  if (cmdLength > 0 && millis() - lastCmdCharMs > 1500) {
    runCommand();
  }
}

void runCommand() {
  cmdBuffer[cmdLength] = '\0';
  cmdLength = 0;
  char letter = tolower(cmdBuffer[0]);
  int number = atoi(cmdBuffer + 1);

  if (letter == 'h') {
    manualMode = true;
    Serial.println(F("HOME: moving to the middle. Tracking paused - type g to resume."));
    moveAtScanSpeed(PAN_HOME, TILT_HOME);
  } else if (letter == 'p' || letter == 't') {
    manualMode = true;
    float limitLow  = (letter == 'p') ? PAN_MIN : TILT_MIN;
    float limitHigh = (letter == 'p') ? PAN_MAX : TILT_MAX;
    // Manual moves may go past the limits on purpose (to find them), but
    // never past what a servo can do.
    float target = clampFloat(number, 0, 180);
    Serial.print(letter == 'p' ? F("PAN") : F("TILT"));
    Serial.print(F(" -> "));
    Serial.print((int)target);
    if (target < limitLow || target > limitHigh) {
      Serial.print(F("  (outside the limits in the code - careful!)"));
    }
    Serial.println(F("   Tracking paused - type g to resume."));
    if (letter == 'p') moveAtScanSpeed(target, tiltPos);
    else               moveAtScanSpeed(panPos, target);
  } else if (letter == 'g' || letter == 'd') {
    manualMode = false;
    scanForLight();
  } else if (letter == 'c') {
    runCalibration();
  } else if (letter == 'r') {
    resetCalibration();
    EEPROM.put(CAL_EEPROM_ADDR, cal);
    Serial.println(F("Calibration reset (no correction)."));
    readAllSensors(true);
  } else {
    Serial.println(F("Commands: h  p90  t45  g  c  r"));
  }
}

// ============================================================================
//  CALIBRATION
// ============================================================================

void loadCalibration() {
  EEPROM.get(CAL_EEPROM_ADDR, cal);
  bool valid = (cal.magic == CAL_MAGIC);
  for (int i = 0; valid && i < 4; i++) {
    if (!(cal.gain[i] >= CAL_MIN_GAIN && cal.gain[i] <= CAL_MAX_GAIN)) valid = false;
  }
  if (!valid) resetCalibration();
}

void resetCalibration() {
  cal.magic = CAL_MAGIC;
  for (int i = 0; i < 4; i++) cal.gain[i] = 1.0;
}

// Measures all four sensors under the SAME light and works out how much to
// scale each one so they all read the same.
void runCalibration() {
  Serial.println();
  Serial.println(F("=== CALIBRATION ==="));
  Serial.println(F("Give all 4 sensors the SAME light: lay a sheet of white paper"));
  Serial.println(F("flat across the top of the + wall and shine the flashlight"));
  Serial.println(F("straight down onto the paper. Hold it steady."));
  countdown();
  Serial.println(F("Measuring - hold still..."));

  float total[4] = {0, 0, 0, 0};
  float rawSum[4] = {0, 0, 0, 0};
  int count = 0;
  unsigned long start = millis();
  while (millis() - start < CAL_MEASURE_MS) {
    for (int i = 0; i < 4; i++) {
      float r = readLdrRaw(LDR_PINS[i]);
      rawSum[i] += r;
      total[i] += brightnessFromRaw(r);
    }
    count++;
    delay(10);
  }

  // Check each sensor's raw reading first.
  for (int i = 0; i < 4; i++) {
    float rawAvg = rawSum[i] / count;
    if (rawAvg < CAL_DEAD_RAW) {
      Serial.print(F("FAILED: sensor "));
      Serial.print(i == 0 ? F("TL") : i == 1 ? F("TR") : i == 2 ? F("BL") : F("BR"));
      Serial.println(F(" reads almost 0 - it's covered, unplugged, or its wire is loose."));
      Serial.println(F("Nothing saved."));
      return;
    }
    if (rawAvg > CAL_MAXED_RAW) {
      Serial.print(F("FAILED: sensor "));
      Serial.print(i == 0 ? F("TL") : i == 1 ? F("TR") : i == 2 ? F("BL") : F("BR"));
      Serial.println(F(" is maxed out (light too bright). Hold the flashlight"));
      Serial.println(F("farther from the paper and try again. Nothing saved."));
      return;
    }
  }

  // Reference = geometric mean, so corrections are balanced (some sensors
  // get turned up, some turned down) instead of all piling onto one.
  float logSum = 0;
  for (int i = 0; i < 4; i++) logSum += log(total[i] / count);
  float reference = exp(logSum / 4.0);

  Calibration newCal;
  newCal.magic = CAL_MAGIC;
  for (int i = 0; i < 4; i++) {
    newCal.gain[i] = clampFloat(reference / (total[i] / count), CAL_MIN_GAIN, CAL_MAX_GAIN);
  }

  // Warn (but still save) if a sensor needed a really big correction.
  for (int i = 0; i < 4; i++) {
    if (newCal.gain[i] > CAL_WARN_GAIN || newCal.gain[i] < 1.0 / CAL_WARN_GAIN) {
      Serial.print(F("Note: sensor "));
      Serial.print(i == 0 ? F("TL") : i == 1 ? F("TR") : i == 2 ? F("BL") : F("BR"));
      Serial.println(F(" needed a big correction. That's OK, but if the light on the"));
      Serial.println(F("  paper wasn't even, redo it. If that sensor's leg was repaired,"));
      Serial.println(F("  a loose joint can make it read low - worth checking."));
    }
  }

  cal = newCal;
  EEPROM.put(CAL_EEPROM_ADDR, cal);
  Serial.println(F("Calibration saved! (It stays saved after power-off.)"));
  printCalibration();
  Serial.println(F("Remove the paper, then type g to scan and track."));
  Serial.println();
  manualMode = true;   // wait for g so it doesn't track the paper
  readAllSensors(true);
}

void printCalibration() {
  Serial.print(F("Calibration (100 = no correction): TL "));
  Serial.print((int)(cal.gain[0] * 100 + 0.5));
  Serial.print(F("  TR "));
  Serial.print((int)(cal.gain[1] * 100 + 0.5));
  Serial.print(F("  BL "));
  Serial.print((int)(cal.gain[2] * 100 + 0.5));
  Serial.print(F("  BR "));
  Serial.println((int)(cal.gain[3] * 100 + 0.5));
}

// ============================================================================
//  DEBUG OUTPUT
// ============================================================================

void printDebug(float vertError, float horizError, const char *status) {
  if (!SERIAL_DEBUG) return;

  unsigned long now = millis();
  if (lastDebugPrint != 0 && now - lastDebugPrint < DEBUG_PRINT_INTERVAL_MS) return;
  lastDebugPrint = now;

  // Raw readings (0 - 1023), same numbers as test mode
  Serial.print(status);
  Serial.print(F(" | TL:"));  Serial.print((int)raw[0]);
  Serial.print(F(" TR:"));    Serial.print((int)raw[1]);
  Serial.print(F(" BL:"));    Serial.print((int)raw[2]);
  Serial.print(F(" BR:"));    Serial.print((int)raw[3]);

  // Errors as percentages: + means light is from the TOP / LEFT
  Serial.print(F(" | up/down:"));    Serial.print((int)(vertError * 100));
  Serial.print(F("% left/right:"));  Serial.print((int)(horizError * 100));
  Serial.print(F("%"));

  Serial.print(F(" | light from: "));
  bool centered = true;
  if (vertError > START_THRESHOLD)        { Serial.print(F("TOP "));    centered = false; }
  else if (vertError < -START_THRESHOLD)  { Serial.print(F("BOTTOM ")); centered = false; }
  if (horizError > START_THRESHOLD)       { Serial.print(F("LEFT"));    centered = false; }
  else if (horizError < -START_THRESHOLD) { Serial.print(F("RIGHT"));   centered = false; }
  if (centered) Serial.print(F("CENTER"));

  if (PANEL_MONITOR) {
    Serial.print(F(" | panel:"));
    Serial.print(panelVolts, 2);
    Serial.print(F("V"));
    if (PANEL_LOAD_OHMS > 0) {
      Serial.print(F(" "));
      Serial.print(panelVolts * panelVolts / PANEL_LOAD_OHMS, 3);
      Serial.print(F("W"));
    }
    Serial.print(F(" (max "));
    Serial.print(panelMaxVolts, 2);
    Serial.print(F(")"));
  }

  Serial.print(F(" | pan:"));  Serial.print((int)(panPos + 0.5));
  Serial.print(F(" tilt:"));   Serial.print((int)(tiltPos + 0.5));
  if (panPos <= PAN_MIN || panPos >= PAN_MAX || tiltPos <= TILT_MIN || tiltPos >= TILT_MAX) {
    Serial.print(F(" (at limit)"));
  }
  Serial.println();
}
