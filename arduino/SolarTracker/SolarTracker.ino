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
 *  The code always moves the panel TOWARD the brighter side, so the shadows
 *  shrink until all four sensors see about the same light.
 *
 *  SERIAL COMMANDS (Serial Monitor, 9600 baud - type the letter, press Enter)
 *    d  = direction test: tells you if TILT_REVERSED / PAN_REVERSED are right
 *    c  = calibrate the sensors so they all read the same under the same light
 *    r  = reset calibration back to "no correction"
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
//  DIRECTION SETTINGS
// ============================================================================
//
//  Easiest way to get these right: upload, open the Serial Monitor, type  d
//  and press Enter, then follow the instructions. The direction test tells
//  you exactly what to set here.
//
//  (Manual way: shine a flashlight from off to one side. If an axis turns
//  AWAY from the light, flip its setting. Test pan with the plate tilted
//  clearly to one side, not flat.)

const bool TILT_REVERSED = false;
const bool PAN_REVERSED  = false;

// Set to true ONLY if your readings go DOWN when you shine light on a sensor.
// (That happens if the LDR and 10k resistor are swapped in the divider.)
const bool INVERT_LDR_READINGS = false;

// ============================================================================
//  SERVO LIMITS  (degrees, 0 - 180)
// ============================================================================
//  Narrow these if the 3D printed parts hit each other or pull on wires near
//  the ends of travel.

const int PAN_MIN  = 0;
const int PAN_MAX  = 180;
const int PAN_HOME = 90;

const int TILT_MIN  = 0;
const int TILT_MAX  = 180;
const int TILT_HOME = 90;

// Tilt servo angle at which the panel faces STRAIGHT UP at the sky (flat).
//
// Why this matters: when the plate tilts past flat to the other side, the
// panel is "upside down" relative to the pan servo, so turning the pan servo
// the same way would swing the panel the OPPOSITE direction. The code flips
// the pan direction when the plate crosses flat so it still turns toward the
// light.
const int  TILT_FLAT_ANGLE    = 90;
const bool FLIP_PAN_PAST_FLAT = true;

// The plate has to go this many degrees PAST flat before the pan direction
// flips. Stops the pan from flip-flopping when the light is nearly overhead.
const float FLAT_HYSTERESIS = 6.0;

// ============================================================================
//  PAN "WRAP-AROUND"  (reaching light that's behind the pan end stops)
// ============================================================================
//  The pan servo only turns 180 degrees. If the light is further around than
//  that, the panel can still point at it by swinging the pan to its OTHER end
//  and flipping the tilt to the other side of flat. This only happens when:
//    - the pan is stuck at an end stop, AND
//    - the light is CLEARLY further that way (error >= WRAP_MIN_ERROR), AND
//    - that has been true for WRAP_CONFIRM_MS without a break.
//  After a wrap it can't wrap again until the pan has moved at least
//  WRAP_REARM_DEGREES away from where it landed, so it can never bounce back
//  and forth between the end stops.
//  Set to false if you'd rather it just wait at the end stop.

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
const float START_THRESHOLD = 0.08;
const float STOP_THRESHOLD  = 0.03;

// How hard to push toward the light: degrees moved per update for each unit
// of error. With GAIN = 4, a 25% error asks for a 1 degree step.
const float GAIN = 4.0;

// Never move more than this many degrees in one update (keeps motion smooth
// and gives the servos and plate time to keep up).
const float MAX_STEP = 1.0;

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

// ============================================================================
//  CALIBRATION / DIRECTION TEST SETTINGS
// ============================================================================

const unsigned long COUNTDOWN_MS   = 5000;  // time to get the light set up
const unsigned long CAL_MEASURE_MS = 2000;  // time spent measuring
const float CAL_MIN_GAIN = 0.4;  // reject calibration if a sensor needs more
const float CAL_MAX_GAIN = 2.5;  // correction than this (probably shaded/broken)
const int   CAL_EEPROM_ADDR = 0;
const uint16_t CAL_MAGIC = 0x5A18;  // marks the EEPROM data as valid

const float DIRTEST_TILT_MOVE  = 12.0;  // how far tilt is nudged each way
const float DIRTEST_PAN_MOVE   = 20.0;  // how far pan is nudged each way
const float DIRTEST_MIN_CHANGE = 0.08;  // smaller change = "can't tell"

// ============================================================================
//  GLOBALS
// ============================================================================

Servo panServo;
Servo tiltServo;

// Positions are kept as floats so small steps add up smoothly.
float panPos  = PAN_HOME;
float tiltPos = TILT_HOME;

// Sensor order everywhere: 0 = TL, 1 = TR, 2 = BL, 3 = BR
const uint8_t LDR_PINS[4] = {
  LDR_TOP_LEFT_PIN, LDR_TOP_RIGHT_PIN, LDR_BOTTOM_LEFT_PIN, LDR_BOTTOM_RIGHT_PIN
};

// Calibration: each sensor's brightness is multiplied by its gain so all four
// read the same under the same light. Saved in EEPROM, so it survives
// power-off and re-uploading the code.
struct Calibration {
  uint16_t magic;
  float gain[4];
};
Calibration cal;

float raw[4];       // latest raw readings, 0 - 1023 (what you saw in test mode)
float smoothed[4];  // smoothed, calibrated brightness used for tracking

bool tiltMoving = false;       // start/stop threshold state for each axis
bool panMoving  = false;
bool tiltOnHighSide = true;    // is the plate past flat toward the high angles?

bool wrapArmed = true;
bool wrapPending = false;
unsigned long wrapPendingSince = 0;
float wrapLandingPos = 0;

unsigned long lastDebugPrint = 0;

// Function prototypes (needed by PlatformIO; harmless in the Arduino IDE)
float readLdrRaw(uint8_t pin);
float brightnessFromRaw(float rawValue);
void  readAllSensors(bool resetSmoothing);
void  computeErrors(const float *b, float &vertError, float &horizError);
void  measureAveragedErrors(float &vertError, float &horizError);
float clampFloat(float value, float low, float high);
float computeStep(float error, bool &moving);
void  writeServos();
void  slewServosTo(float newPan, float newTilt, unsigned long durationMs);
void  countdown();
void  handleSerialCommands();
void  loadCalibration();
void  resetCalibration();
void  runCalibration();
void  printCalibration();
void  runDirectionTest();
void  printDebug(float vertError, float horizError, const char *status);

// ============================================================================
//  SETUP
// ============================================================================

void setup() {
  Serial.begin(9600);

  panServo.attach(PAN_SERVO_PIN);
  tiltServo.attach(TILT_SERVO_PIN);
  writeServos();

  Serial.println(F("Solar tracker starting..."));
  Serial.println(F("(If you see this line again while it's running, the Arduino"));
  Serial.println(F(" reset - usually a servo power problem. See README.)"));

  loadCalibration();
  printCalibration();
  Serial.println(F("Commands: d = direction test, c = calibrate, r = reset calibration"));

  if (SENSOR_TEST_MODE) {
    Serial.println(F("*** SENSOR TEST MODE: servos will NOT move ***"));
    Serial.println(F("Shine a light on each sensor - its number should go UP."));
  }

  tiltOnHighSide = (tiltPos >= TILT_FLAT_ANGLE);

  delay(1000);  // give the servos time to reach home
  readAllSensors(true);
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

  float rawAvg = (raw[0] + raw[1] + raw[2] + raw[3]) / 4.0;
  if (rawAvg < DARK_THRESHOLD) {
    tiltMoving = panMoving = false;
    wrapPending = false;
    printDebug(vertError, horizError, "DARK-HOLD");
    delay(LOOP_INTERVAL_MS);
    return;
  }

  // ---- 4. TILT: tip the panel toward the brighter (top or bottom) side ----
  // Default assumption: a bigger tilt angle tips the panel face toward the
  // TOP sensors. TILT_REVERSED flips this.
  float tiltDir = TILT_REVERSED ? -1.0 : 1.0;
  tiltPos += tiltDir * computeStep(vertError, tiltMoving);
  tiltPos = clampFloat(tiltPos, TILT_MIN, TILT_MAX);

  // Which side of flat is the plate on? It has to go FLAT_HYSTERESIS degrees
  // past flat before this changes, so the pan doesn't flip-flop near flat.
  if (tiltPos > TILT_FLAT_ANGLE + FLAT_HYSTERESIS) tiltOnHighSide = true;
  if (tiltPos < TILT_FLAT_ANGLE - FLAT_HYSTERESIS) tiltOnHighSide = false;

  // ---- 5. PAN: turn the panel toward the brighter (left or right) side ----
  // Default assumption: while the plate is on the high-angle side of flat,
  // a bigger pan angle swings the panel face toward the LEFT sensors.
  // PAN_REVERSED flips this. On the other side of flat the effect of the pan
  // servo is mirrored, so the direction is flipped again.
  float panDir = PAN_REVERSED ? -1.0 : 1.0;
  if (FLIP_PAN_PAST_FLAT && !tiltOnHighSide) {
    panDir = -panDir;
  }
  float panStep = panDir * computeStep(horizError, panMoving);

  // ---- 6. Pan is at an end stop but the light is clearly further around ----
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

  // ---- 7. Send the new positions to the servos ----
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
float brightnessFromRaw(float rawValue) {
  if (rawValue > 1023) rawValue = 1023;
  if (rawValue < 0)    rawValue = 0;
  return rawValue / (1023.5 - rawValue);
}

// Reads all four sensors, applies calibration, and smooths them over time.
// resetSmoothing = true throws away the old history (used after big moves).
void readAllSensors(bool resetSmoothing) {
  for (int i = 0; i < 4; i++) {
    raw[i] = readLdrRaw(LDR_PINS[i]);
    float value = brightnessFromRaw(raw[i]) * cal.gain[i];
    if (resetSmoothing) {
      smoothed[i] = value;
    } else {
      smoothed[i] += SMOOTHING * (value - smoothed[i]);
    }
  }
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

// Averages the sensors for half a second and returns the errors (used by
// the direction test, where accuracy matters more than speed).
void measureAveragedErrors(float &vertError, float &horizError) {
  float total[4] = {0, 0, 0, 0};
  const int count = 25;
  for (int n = 0; n < count; n++) {
    for (int i = 0; i < 4; i++) {
      total[i] += brightnessFromRaw(readLdrRaw(LDR_PINS[i])) * cal.gain[i];
    }
    delay(20);
  }
  computeErrors(total, vertError, horizError);
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
void slewServosTo(float newPan, float newTilt, unsigned long durationMs) {
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
  delay(300);  // let the plate stop wobbling
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

void handleSerialCommands() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'c' || c == 'C') {
      runCalibration();
    } else if (c == 'd' || c == 'D') {
      runDirectionTest();
    } else if (c == 'r' || c == 'R') {
      resetCalibration();
      EEPROM.put(CAL_EEPROM_ADDR, cal);
      Serial.println(F("Calibration reset (no correction)."));
      readAllSensors(true);
    }
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
  float rawTotal = 0;
  int count = 0;
  unsigned long start = millis();
  while (millis() - start < CAL_MEASURE_MS) {
    for (int i = 0; i < 4; i++) {
      float r = readLdrRaw(LDR_PINS[i]);
      rawTotal += r;
      total[i] += brightnessFromRaw(r);
    }
    count++;
    delay(10);
  }

  if (rawTotal / (4.0 * count) < DARK_THRESHOLD * 2) {
    Serial.println(F("FAILED: too dark. Use a brighter light and try again."));
    return;
  }

  float mean = (total[0] + total[1] + total[2] + total[3]) / 4.0;
  Calibration newCal;
  newCal.magic = CAL_MAGIC;
  for (int i = 0; i < 4; i++) {
    newCal.gain[i] = (total[i] > 0) ? mean / total[i] : 0;
    if (newCal.gain[i] < CAL_MIN_GAIN || newCal.gain[i] > CAL_MAX_GAIN) {
      Serial.print(F("FAILED: sensor "));
      Serial.print(i == 0 ? F("TL") : i == 1 ? F("TR") : i == 2 ? F("BL") : F("BR"));
      Serial.println(F(" is way off from the others. Is it shaded, or the light"));
      Serial.println(F("not even? Check its wiring, then try again. Nothing saved."));
      return;
    }
  }

  cal = newCal;
  EEPROM.put(CAL_EEPROM_ADDR, cal);
  Serial.println(F("Calibration saved! (It stays saved after power-off.)"));
  printCalibration();
  Serial.println();
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
//  DIRECTION TEST
// ============================================================================
//  Nudges each servo a little both ways and watches what happens to the
//  shadows. Moving the right way makes the error go DOWN, whichever side the
//  light is on, so this works no matter where exactly the light is held.

void runDirectionTest() {
  Serial.println();
  Serial.println(F("=== DIRECTION TEST ==="));

  // Test pose: plate tilted 45 degrees to the high side of flat, pan centered
  // with room to move both ways.
  float posePan  = clampFloat(PAN_HOME, PAN_MIN + DIRTEST_PAN_MOVE, PAN_MAX - DIRTEST_PAN_MOVE);
  float poseTilt = clampFloat(TILT_FLAT_ANGLE + 45, TILT_MIN + DIRTEST_TILT_MOVE, TILT_MAX - DIRTEST_TILT_MOVE);
  slewServosTo(posePan, poseTilt, 1500);

  Serial.println(F("The panel is now tilted to one side. Hold the flashlight"));
  Serial.println(F("STEADY 1-2 feet away, shining STRAIGHT AT THE PANEL FACE"));
  Serial.println(F("(from the direction it's pointing). Don't follow the panel"));
  Serial.println(F("when it moves - keep the light still until it says DONE."));
  countdown();

  float vMinus, vPlus, hMinus, hPlus, unused;

  // --- Tilt ---
  slewServosTo(posePan, poseTilt - DIRTEST_TILT_MOVE, 600);
  measureAveragedErrors(vMinus, unused);
  slewServosTo(posePan, poseTilt + DIRTEST_TILT_MOVE, 1200);
  measureAveragedErrors(vPlus, unused);
  slewServosTo(posePan, poseTilt, 600);

  // --- Pan ---
  slewServosTo(posePan - DIRTEST_PAN_MOVE, poseTilt, 600);
  measureAveragedErrors(unused, hMinus);
  slewServosTo(posePan + DIRTEST_PAN_MOVE, poseTilt, 1200);
  measureAveragedErrors(unused, hPlus);
  slewServosTo(posePan, poseTilt, 600);

  Serial.println(F("DONE. Results:"));

  // Tilt: if a bigger angle tips the face toward the TOP sensors, the
  // top-minus-bottom error must DROP as the angle goes up.
  float tiltChange = vPlus - vMinus;
  Serial.print(F("  TILT (error change "));
  Serial.print((int)(tiltChange * 100));
  Serial.print(F("%): "));
  if (fabs(tiltChange) < DIRTEST_MIN_CHANGE) {
    Serial.println(F("can't tell - aim the light straight at the panel, closer, and retry."));
  } else {
    bool shouldBeReversed = (tiltChange > 0);
    Serial.print(F("set  TILT_REVERSED = "));
    Serial.print(shouldBeReversed ? F("true") : F("false"));
    Serial.println(shouldBeReversed == TILT_REVERSED ? F("   (already correct)")
                                                     : F("   <-- CHANGE THIS and re-upload"));
  }

  // Pan: on the high side of flat, if a bigger angle swings the face toward
  // the LEFT sensors, the left-minus-right error must DROP as the angle goes up.
  float panChange = hPlus - hMinus;
  if (poseTilt < TILT_FLAT_ANGLE) panChange = -panChange;  // other side of flat
  Serial.print(F("  PAN  (error change "));
  Serial.print((int)(panChange * 100));
  Serial.print(F("%): "));
  if (fabs(panChange) < DIRTEST_MIN_CHANGE) {
    Serial.println(F("can't tell - aim the light straight at the panel, closer, and retry."));
  } else {
    bool shouldBeReversed = (panChange > 0);
    Serial.print(F("set  PAN_REVERSED  = "));
    Serial.print(shouldBeReversed ? F("true") : F("false"));
    Serial.println(shouldBeReversed == PAN_REVERSED ? F("   (already correct)")
                                                    : F("   <-- CHANGE THIS and re-upload"));
  }
  Serial.println();

  tiltOnHighSide = (tiltPos >= TILT_FLAT_ANGLE);
  tiltMoving = panMoving = false;
  readAllSensors(true);
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

  Serial.print(F(" | pan:"));  Serial.print((int)(panPos + 0.5));
  Serial.print(F(" tilt:"));   Serial.println((int)(tiltPos + 0.5));
}
