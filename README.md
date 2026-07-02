# Line Following Robot 🏎️⚡

> **LFR TECHATHON NATIONALISTS 2026** — organized by **IUT Robotics Society**

Our entry for the LFR Techathon Nationalists 2026. PID-controlled autonomous line follower — follows a black line on white surface, goal is simple: reach the finish before everyone else. Speed is everything.

Built from scratch — custom chassis, hand-soldered electronics, and code we iterated on until it stopped embarrassing us on the track.

## The Robot

```
          FRONT (caster ball)
       ┌──────────────────────┐
       │  [S0][S1][S2][S3][S4]│  ← 5-channel IR sensor array
       │         ___          │
       │        (   )         │  ← caster ball (free wheel)
       │         ‾‾‾          │
       │                      │
       │   ┌──────────────┐   │
       │   │  Arduino Nano│   │
       │   └──────────────┘   │
       │   ┌──────────────┐   │
       │   │  L298N Motor │   │
       │   │    Driver    │   │
       │   └──────────────┘   │
       │   ┌──────────────┐   │
       │   │ LM2596 Buck  │   │
       │   │  Converter   │   │
       │   └──────────────┘   │
       │  [18650] [18650]     │  ← 2x Li-ion batteries
       │                      │
  ═══╤╧╤══════════════════╤╧╤═══
  ▓▓▓│L│                  │R│▓▓▓  ← drive wheels (motors)
  ═══╧╤╧══════════════════╧╤╧═══
          REAR
```

## How It Works

### Sensor Reading

The 5 IR sensors sit at the front, facing the ground. Each sensor returns a value — high when it sees the black line, low on white surface. We read all 5 and pack them into a 5-bit number:

```
                Black Line
                    ║
  Sensor:   S0   S1  ║S2   S3   S4
            ○    ○   ●    ○    ○       ← S2 sees the line
                     ║
  Bit:      0    0   1    0    0       = B00100 (line is centered)
```

Some more examples:

```
  Line drifted left:
  S0   S1   S2   S3   S4
  ○    ●    ●    ○    ○    = B01100 → error is negative → turn left

  Line drifted right:
  S0   S1   S2   S3   S4
  ○    ○    ○    ●    ○    = B00010 → error is positive → turn right

  Line completely lost:
  S0   S1   S2   S3   S4
  ○    ○    ○    ○    ○    = B00000 → spin in place to recover

  Junction / cross:
  S0   S1   S2   S3   S4
  ●    ●    ●    ●    ●    = B11111 → just go straight through
```

### PID Control

Instead of doing a bunch of `if-else` checks for each pattern, we use PID (Proportional-Integral-Derivative) control. Here's the idea:

We assign weights to each sensor based on their position:

```
  Sensor:    S0     S1     S2     S3     S4
  Weight:    -2     -1      0     +1     +2
             ←  LEFT    CENTER    RIGHT  →
```

We calculate a **weighted average** of all the sensors that see the line. This gives us an **error value** from -2.0 (far left) to +2.0 (far right).

```
  Example: S1 and S2 see the line
  Weighted sum = (-1) + (0) = -1
  Active sensors = 2
  Error = -1 / 2 = -0.5  →  line is slightly to the left
```

Then PID kicks in:

```
  correction = Kp × error  +  Ki × integral  +  Kd × derivative
                   │                │                   │
                   │                │                   └─ dampens oscillation
                   │                └─ fixes long-term drift
                   └─ main steering force

  Left motor speed  = BASE_SPEED - correction
  Right motor speed = BASE_SPEED + correction
```

So if error is negative (line to the left), correction is negative, left motor slows down, robot turns left. Simple.

```
  Line to the left (error = -1.5):
  ┌─────┐         ┌─────┐
  │ LEFT│ SLOW    │RIGHT│ FAST     → robot turns LEFT ↰
  │motor│ ←──     │motor│ ──→
  └─────┘         └─────┘

  Line centered (error = 0):
  ┌─────┐         ┌─────┐
  │ LEFT│ ──→     │RIGHT│ ──→     → robot goes STRAIGHT ↑
  │motor│         │motor│
  └─────┘         └─────┘

  Line to the right (error = +1.5):
  ┌─────┐         ┌─────┐
  │ LEFT│ FAST    │RIGHT│ SLOW     → robot turns RIGHT ↱
  │motor│ ──→     │motor│ ←──
  └─────┘         └─────┘
```

### What Happens When the Line is Lost?

If all 5 sensors read white (no line), we remember which direction we last turned and spin in place in that direction until we find the line again:

```
  Lost line, last turn was LEFT:       Lost line, last turn was RIGHT:

      ╔═══╗                                 ╔═══╗
      ║   ║                                 ║   ║
  ←── ║   ║ ──→                         ←── ║   ║ ──→
  REV ╚═══╝ FWD                         FWD ╚═══╝ REV
    spin LEFT ↺                           spin RIGHT ↻
```

## Wiring

```
  Arduino Pin    →    Component
  ─────────────────────────────────
  A0             →    IR Sensor 0 (leftmost)
  A1             →    IR Sensor 1
  A2             →    IR Sensor 2 (center)
  A3             →    IR Sensor 3
  A4             →    IR Sensor 4 (rightmost)
  D5  (PWM)      →    L298N ENA (left motor speed)
  D6             →    L298N IN1 (left motor forward)
  D7             →    L298N IN2 (left motor backward)
  D8             →    L298N IN3 (right motor forward)
  D9             →    L298N IN4 (right motor backward)
  D10 (PWM)      →    L298N ENB (right motor speed)
```

## File Structure

```
Sensor/
├── Sensor.ino    — main file: setup(), loop(), PID logic, global variables
├── motor.ino     — setMotor(): handles direction and PWM for both wheels
├── reading.ino   — readSensors(): reads IR array, debugPrint(): serial output
└── README.md     — you're reading this
```

Arduino IDE automatically merges all `.ino` files in the same folder into one sketch before compiling. The file matching the folder name (`Sensor.ino`) is treated as the main file.

## Tuning Guide

These are the values you'll want to tweak on the actual track:

| Parameter | Default | What it does |
|-----------|---------|-------------|
| `threshold` | 512 | Sensor cutoff — values above this = "line detected". Check with `debugPrint()`. |
| `BASE_SPEED` | 180 | Forward speed. Higher = faster but harder to control on curves. |
| `Kp` | 30.0 | How aggressively it steers. Too low = misses turns. Too high = wobbles. |
| `Kd` | 20.0 | Dampens the wobble. Increase if robot oscillates side to side. |
| `Ki` | 0.0 | Fixes drift. Only touch this if the robot consistently veers to one side. |

### Tuning steps

1. Start with `BASE_SPEED = 150`, `Kp = 25`, `Kd = 15`, `Ki = 0`
2. Put robot on track, see if it follows the line
3. If it doesn't turn enough on curves → increase `Kp`
4. If it wobbles/oscillates → increase `Kd`
5. Once stable, slowly increase `BASE_SPEED`
6. Comment out `debugPrint()` for final competition run — it slows the loop

## Hardware

- **Microcontroller**: Arduino Nano (ATmega328P)
- **Sensors**: 5-channel IR sensor array (analog output)
- **Motor Driver**: L298N dual H-bridge
- **Motors**: 2× DC gear motors (rear wheels)
- **Power**: 2× 18650 Li-ion batteries → LM2596 buck converter → regulated 5V
- **Chassis**: Custom laser-cut wood frame with front caster ball

## Competition

**LFR TECHATHON NATIONALISTS 2026**\
Organized by **IUT Robotics Society**

Format: autonomous line following race — black line on white surface, fastest time wins. No remote control, no manual intervention. Just your bot, your code, and the track.

## Team — NSU

| Name | Role |
|------|------|
| **Mahin Ahmad** | Code & PID tuning |
| **Faiyaz Tahsin** | Circuit & wiring |
| **Ismail Hossain** | Chassis & hardware |
| **Rimajul Islam Shihab** | Testing & calibration |

Four CSE undergrads from **North South University** who spend way too much time soldering things at 3 AM and arguing about PID gains. We got into robotics because writing code that moves things in the real world hits different than another CRUD app.

If you're from the competition and reading this — see you on the track. 🏁
