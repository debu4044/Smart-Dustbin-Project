# Version 1 - Basic Smart Dustbin (Semester 4 Mini Project)

## Overview

This is the original Smart Dustbin developed as our Semester 4 Mini Project.

The objective was to design a low-cost touchless dustbin capable of automatically opening its lid when a user approached while also preventing operation when the dustbin was full.

The implementation was intentionally kept simple due to hardware availability and academic constraints.

---

## Hardware

- ESP32
- 2 × HC-SR04 Ultrasonic Sensors
- Servo Motor
- Breadboard
- Jumper Wires

---

## Features

- Automatic lid opening
- Automatic lid closing
- Fill level detection
- Overflow prevention
- Simple procedural implementation

---

## Working

Sensor 1 detects an approaching user.

↓

Servo opens the lid.

↓

User disposes waste.

↓

Lid closes automatically.

↓

Sensor 2 continuously monitors the garbage level.

↓

If the bin becomes full, automatic lid operation is suspended.

---

## Limitations

- No IoT
- No notifications
- No cloud storage
- Basic threshold-based detection

---

This version served as the foundation for all future improvements.
