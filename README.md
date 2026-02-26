# Arduino Stepper Motor Positioning System

A complete stepper motor position controller with absolute/relative movement, speed control, homing sequence, and a Serial command interface.

## Hardware Required

| Component | Detail |
|-----------|--------|
| Arduino Uno / Nano | Microcontroller |
| NEMA 17 Stepper | 1.8° per step (200 steps/rev) |
| DRV8825 or A4988 | Stepper motor driver |
| Limit switch | Homing reference |
| 12–24V Power Supply | Motor power (separate from Arduino 5V) |

## Wiring

```
STEP   ──► Pin 3
DIR    ──► Pin 4
EN     ──► Pin 5  (active LOW)
LIMIT  ──► Pin 2  (normally open, to GND)
```

## Serial Commands

Open Serial Monitor at **9600 baud**:

| Command | Description |
|---------|-------------|
| `MOVE 400` | Move 400 steps forward (relative) |
| `MOVE -200` | Move 200 steps backward |
| `GOTO 800` | Move to absolute step position 800 |
| `HOME` | Run homing sequence, reset position to 0 |
| `SPEED 1000` | Set step delay to 1000 µs (faster) |
| `STATUS` | Print current position info |

## Example Session

```
Stepper Motor Positioning System Ready.
> HOME
Homing...
Home found. Position reset to 0.

> MOVE 400
Moving 400 steps FORWARD...
Done. Position: 400

> STATUS
--- Motor Status ---
  Position (steps)  : 400
  Position (revs)   : 2
  Angle             : 0.0°
  Step delay        : 1500 us
--------------------
```

## Key Concepts Demonstrated

- Stepper motor open-loop control
- Absolute and relative positioning
- Limit switch homing
- Serial command parsing / state management
- Driver enable/disable for thermal management
