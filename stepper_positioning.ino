/*
 * Stepper Motor Positioning System
 * Author: [Your Name]
 *
 * Description:
 *   Controls a NEMA 17 stepper motor with a DRV8825 or A4988 driver.
 *   Supports absolute and relative positioning, speed control,
 *   homing with a limit switch, and Serial command interface.
 *
 * Hardware:
 *   - Arduino Uno / Nano
 *   - NEMA 17 Stepper Motor (1.8° per step = 200 steps/rev)
 *   - DRV8825 or A4988 Stepper Driver
 *   - Limit switch (for homing)
 *   - 12–24V power supply for motor
 *
 * Wiring:
 *   - STEP  → Pin 3
 *   - DIR   → Pin 4
 *   - EN    → Pin 5  (active LOW)
 *   - LIMIT → Pin 2  (INPUT_PULLUP, active LOW)
 *
 * Serial Commands:
 *   MOVE <steps>    — move relative steps (+ forward, - backward)
 *   GOTO <steps>    — move to absolute position
 *   HOME            — run homing sequence
 *   SPEED <us>      — set step delay in microseconds
 *   STATUS          — print current position and speed
 */

// ── Pin Definitions ──────────────────────────────────────────────────────────
const int STEP_PIN  = 3;
const int DIR_PIN   = 4;
const int EN_PIN    = 5;
const int LIMIT_PIN = 2;

// ── Motor Parameters ─────────────────────────────────────────────────────────
const int STEPS_PER_REV  = 200;      // 1.8° stepper = 200 full steps
const int MICROSTEP_MODE = 1;        // 1 = full step (adjust for driver config)

// ── Speed / Position ─────────────────────────────────────────────────────────
long  current_position  = 0;         // Absolute position in steps
long  target_position   = 0;
int   step_delay_us     = 1500;      // Microseconds between steps (lower = faster)
const int MIN_DELAY_US  = 500;
const int MAX_DELAY_US  = 10000;

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(STEP_PIN,  OUTPUT);
  pinMode(DIR_PIN,   OUTPUT);
  pinMode(EN_PIN,    OUTPUT);
  pinMode(LIMIT_PIN, INPUT_PULLUP);

  disable_motor();   // Don't energize until needed

  Serial.println("Stepper Motor Positioning System Ready.");
  Serial.println("Commands: MOVE <n>, GOTO <n>, HOME, SPEED <us>, STATUS");
}

// ── Main Loop ─────────────────────────────────────────────────────────────────
void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    handle_command(cmd);
  }
}

// ── Command Parser ────────────────────────────────────────────────────────────
void handle_command(String cmd) {
  if (cmd.startsWith("MOVE ")) {
    long steps = cmd.substring(5).toInt();
    move_relative(steps);

  } else if (cmd.startsWith("GOTO ")) {
    long pos = cmd.substring(5).toInt();
    move_to(pos);

  } else if (cmd == "HOME") {
    home_sequence();

  } else if (cmd.startsWith("SPEED ")) {
    int spd = cmd.substring(6).toInt();
    if (spd >= MIN_DELAY_US && spd <= MAX_DELAY_US) {
      step_delay_us = spd;
      Serial.print("Speed set: "); Serial.print(step_delay_us); Serial.println(" us/step");
    } else {
      Serial.println("Speed out of range (500–10000 us)");
    }

  } else if (cmd == "STATUS") {
    print_status();

  } else {
    Serial.println("Unknown command. Use: MOVE, GOTO, HOME, SPEED, STATUS");
  }
}

// ── Motion Functions ─────────────────────────────────────────────────────────

void move_relative(long steps) {
  long target = current_position + steps;
  move_to(target);
}

void move_to(long target) {
  enable_motor();

  long steps_to_move = target - current_position;
  if (steps_to_move == 0) {
    Serial.println("Already at target.");
    return;
  }

  set_direction(steps_to_move > 0);

  long abs_steps = abs(steps_to_move);
  Serial.print("Moving "); Serial.print(abs_steps);
  Serial.println(steps_to_move > 0 ? " steps FORWARD..." : " steps BACKWARD...");

  for (long i = 0; i < abs_steps; i++) {
    step_once();
    if (steps_to_move > 0) current_position++;
    else current_position--;
  }

  Serial.print("Done. Position: "); Serial.println(current_position);
  disable_motor();
}

void home_sequence() {
  Serial.println("Homing...");
  enable_motor();
  set_direction(false);   // Move toward limit switch (negative direction)

  while (digitalRead(LIMIT_PIN) == HIGH) {
    step_once();
    current_position--;
    if (current_position < -10000) {
      Serial.println("ERROR: Limit switch not found. Stopping.");
      disable_motor();
      return;
    }
  }

  current_position = 0;
  target_position  = 0;
  Serial.println("Home found. Position reset to 0.");
  disable_motor();
}

// ── Low-Level Motor Control ───────────────────────────────────────────────────

void step_once() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(step_delay_us);
}

void set_direction(bool forward) {
  digitalWrite(DIR_PIN, forward ? HIGH : LOW);
  delayMicroseconds(5);   // Direction setup time
}

void enable_motor() {
  digitalWrite(EN_PIN, LOW);    // Active LOW
  delayMicroseconds(100);
}

void disable_motor() {
  digitalWrite(EN_PIN, HIGH);   // Disable to reduce heat/current
}

// ── Status ────────────────────────────────────────────────────────────────────
void print_status() {
  float degrees = (current_position % STEPS_PER_REV) * (360.0 / STEPS_PER_REV);
  long  revs    = current_position / STEPS_PER_REV;

  Serial.println("\n--- Motor Status ---");
  Serial.print("  Position (steps)  : "); Serial.println(current_position);
  Serial.print("  Position (revs)   : "); Serial.println(revs);
  Serial.print("  Angle             : "); Serial.print(degrees, 1); Serial.println("°");
  Serial.print("  Step delay        : "); Serial.print(step_delay_us); Serial.println(" us");
  Serial.println("--------------------\n");
}
