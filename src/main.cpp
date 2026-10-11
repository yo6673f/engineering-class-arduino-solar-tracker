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
 *  Every loop the code always moves the panel TOWARD the brighter side, so
 *  the shadows shrink until all four sensors see about the same light.
 *
 *  IMPORTANT: This file and arduino/SolarTracker/SolarTracker.ino are meant to
 *  be IDENTICAL. If you change one, copy the change into the other.
 *
 *  See README.md for wiring, first-run tests, and tuning.
 * ============================================================================
 */

#include <Arduino.h>
#include <Servo.h>

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
//  DIRECTION SETTINGS  (the first thing to check with a flashlight!)
// ============================================================================
//
//  Shine a flashlight at the panel from off to one side. The panel should turn
//  TOWARD the light. If one axis turns AWAY from the light, flip that axis's
//  setting below (false -> true, or true -> false) and re-upload.
//
//  Test the PAN axis with the plate tilted clearly to one side (not flat).
//  You only need to fix it once - the code handles the other side for you.

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
// the same way would swing the panel the OPPOSITE direction. Without this
// correction the tracker would chase AWAY from the light on the pan axis
// whenever it was tilted past flat.
const int  TILT_FLAT_ANGLE    = 90;
const bool FLIP_PAN_PAST_FLAT = true;

// When the pan servo hits an end stop but the light is still further around,
// swing to the "mirror image" position (pan 180 degrees the other way, tilt
// flipped to the other side of flat), which points the panel the same way,
// and keep tracking from there. Lets the tracker follow light from any
// direction even though the pan servo only turns 180 degrees.
const bool WRAP_PAN_AT_LIMITS = true;
const unsigned long WRAP_SWING_DELAY_MS = 1000;

// ============================================================================
//  TUNING
// ============================================================================

// Ignore differences smaller than this fraction (0.05 = 5%). Bigger = steadier
// but less precise. Smaller = more precise but may jitter back and forth.
const float DEADBAND = 0.05;

// How hard to push toward the light: degrees moved per loop for each unit of
// error. The error is a ratio between -1 and +1, so with GAIN = 20 a 10%
// difference asks for a 2 degree step.
const float GAIN = 20.0;

// Never move more than this many degrees in a single loop (keeps motion smooth).
const float MAX_STEP = 2.0;

// If the average reading (0 - 1023) drops below this, it's too dark to track
// (night / covered), so hold position instead of wandering.
const int DARK_THRESHOLD = 30;

// Number of analogRead() samples averaged per sensor (reduces noise).
const int SAMPLES_PER_READ = 8;

// Pause between control loops, in milliseconds.
const unsigned long LOOP_DELAY_MS = 25;

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
//  GLOBALS
// ============================================================================

Servo panServo;
Servo tiltServo;

// Positions are kept as floats so small steps add up smoothly.
float panPos  = PAN_HOME;
float tiltPos = TILT_HOME;

unsigned long lastDebugPrint = 0;

// Function prototypes (needed by PlatformIO; harmless in the Arduino IDE)
int   readLdr(uint8_t pin);
float clampFloat(float value, float low, float high);
float computeStep(float error);
void  printDebug(int tl, int tr, int bl, int br,
                 float vertError, float horizError, const char *status);

// ============================================================================
//  SETUP
// ============================================================================

void setup() {
  Serial.begin(9600);

  panServo.attach(PAN_SERVO_PIN);
  tiltServo.attach(TILT_SERVO_PIN);

  panServo.write(PAN_HOME);
  tiltServo.write(TILT_HOME);

  Serial.println(F("Solar tracker starting..."));
  if (SENSOR_TEST_MODE) {
    Serial.println(F("*** SENSOR TEST MODE: servos will NOT move ***"));
    Serial.println(F("Shine a light on each sensor - its number should go UP."));
  }

  delay(1000);  // give the servos time to reach home
}

// ============================================================================
//  MAIN LOOP
// ============================================================================

void loop() {
  // ---- 1. Read all four sensors (bigger number = more light) ----
  int tl = readLdr(LDR_TOP_LEFT_PIN);
  int tr = readLdr(LDR_TOP_RIGHT_PIN);
  int bl = readLdr(LDR_BOTTOM_LEFT_PIN);
  int br = readLdr(LDR_BOTTOM_RIGHT_PIN);

  // ---- 2. Average each side ----
  float top    = (tl + tr) / 2.0;
  float bottom = (bl + br) / 2.0;
  float left   = (tl + bl) / 2.0;
  float right  = (tr + br) / 2.0;
  float avg    = (tl + tr + bl + br) / 4.0;

  // ---- 3. Normalized errors, each between -1 and +1 ----
  // Dividing by the total makes the tracker behave the same in dim light
  // and bright sunlight.
  //   vertError  > 0  -> light is coming from the TOP
  //   horizError > 0  -> light is coming from the LEFT
  float vertError  = (top + bottom > 0) ? (top - bottom) / (top + bottom) : 0;
  float horizError = (left + right > 0) ? (left - right) / (left + right) : 0;

  // ---- 4. Decide whether to move ----
  if (SENSOR_TEST_MODE) {
    printDebug(tl, tr, bl, br, vertError, horizError, "TEST");
    delay(LOOP_DELAY_MS);
    return;
  }

  if (avg < DARK_THRESHOLD) {
    printDebug(tl, tr, bl, br, vertError, horizError, "DARK-HOLD");
    delay(LOOP_DELAY_MS);
    return;
  }

  // ---- 5. TILT: tip the panel toward the brighter (top or bottom) side ----
  // Default assumption: a bigger tilt angle tips the panel face toward the
  // TOP sensors. TILT_REVERSED flips this.
  float tiltDir = TILT_REVERSED ? -1.0 : 1.0;
  tiltPos += tiltDir * computeStep(vertError);
  tiltPos = clampFloat(tiltPos, TILT_MIN, TILT_MAX);

  // ---- 6. PAN: turn the panel toward the brighter (left or right) side ----
  // Default assumption: while the tilt angle is at or above TILT_FLAT_ANGLE,
  // a bigger pan angle swings the panel face toward the LEFT sensors.
  // PAN_REVERSED flips this. Once the plate tilts past flat, the effect of
  // the pan servo is mirrored, so the direction is flipped again.
  float panDir = PAN_REVERSED ? -1.0 : 1.0;
  if (FLIP_PAN_PAST_FLAT && tiltPos < TILT_FLAT_ANGLE) {
    panDir = -panDir;
  }
  float panStep = panDir * computeStep(horizError);

  // ---- 7. Pan is at its end stop but the light is still further around ----
  // A 180 degree servo can't keep turning, but the panel can point the exact
  // same way by swinging the pan 180 degrees to its other end and mirroring
  // the tilt to the other side of flat. From there it keeps following the
  // light, so the tracker can reach light from ANY direction.
  bool panAtMax = panPos >= PAN_MAX && panStep > 0;
  bool panAtMin = panPos <= PAN_MIN && panStep < 0;
  if (WRAP_PAN_AT_LIMITS && (panAtMax || panAtMin)) {
    panPos  = clampFloat(panAtMax ? panPos - 180 : panPos + 180, PAN_MIN, PAN_MAX);
    tiltPos = clampFloat(2 * TILT_FLAT_ANGLE - tiltPos, TILT_MIN, TILT_MAX);
    panServo.write((int)(panPos + 0.5));
    tiltServo.write((int)(tiltPos + 0.5));
    lastDebugPrint = 0;  // make sure this event gets printed
    printDebug(tl, tr, bl, br, vertError, horizError, "WRAP");
    delay(WRAP_SWING_DELAY_MS);  // let the servos finish the big swing
    return;
  }

  panPos = clampFloat(panPos + panStep, PAN_MIN, PAN_MAX);

  // ---- 8. Send the new positions to the servos ----
  panServo.write((int)(panPos + 0.5));
  tiltServo.write((int)(tiltPos + 0.5));

  printDebug(tl, tr, bl, br, vertError, horizError, "TRACK");
  delay(LOOP_DELAY_MS);
}

// ============================================================================
//  HELPER FUNCTIONS
// ============================================================================

// Reads one photoresistor, averaged over several samples.
// Returns 0 - 1023 where a BIGGER number always means MORE light.
int readLdr(uint8_t pin) {
  analogRead(pin);  // throw away first read after switching pins (more accurate)

  long total = 0;
  for (int i = 0; i < SAMPLES_PER_READ; i++) {
    total += analogRead(pin);
  }
  int value = total / SAMPLES_PER_READ;

  if (INVERT_LDR_READINGS) {
    value = 1023 - value;
  }
  return value;
}

float clampFloat(float value, float low, float high) {
  if (value < low)  return low;
  if (value > high) return high;
  return value;
}

// Turns an error (-1 to +1) into a step size in degrees.
// Returns 0 inside the deadband so the tracker sits still when it's lined up.
float computeStep(float error) {
  if (fabs(error) < DEADBAND) {
    return 0;
  }
  return clampFloat(error * GAIN, -MAX_STEP, MAX_STEP);
}

void printDebug(int tl, int tr, int bl, int br,
                float vertError, float horizError, const char *status) {
  if (!SERIAL_DEBUG) return;

  unsigned long now = millis();
  if (now - lastDebugPrint < DEBUG_PRINT_INTERVAL_MS) return;
  lastDebugPrint = now;

  Serial.print(status);
  Serial.print(F(" | TL:"));  Serial.print(tl);
  Serial.print(F(" TR:"));    Serial.print(tr);
  Serial.print(F(" BL:"));    Serial.print(bl);
  Serial.print(F(" BR:"));    Serial.print(br);

  Serial.print(F(" | light from: "));
  bool centered = true;
  if (vertError > DEADBAND)        { Serial.print(F("TOP "));    centered = false; }
  else if (vertError < -DEADBAND)  { Serial.print(F("BOTTOM ")); centered = false; }
  if (horizError > DEADBAND)       { Serial.print(F("LEFT"));    centered = false; }
  else if (horizError < -DEADBAND) { Serial.print(F("RIGHT"));   centered = false; }
  if (centered) Serial.print(F("CENTER"));

  Serial.print(F(" | pan:"));  Serial.print((int)(panPos + 0.5));
  Serial.print(F(" tilt:"));   Serial.println((int)(tiltPos + 0.5));
}
