🗑️ Smart Dustbin

A simple semester mini-project that slowly refused to stay simple.

What started as...

"Let's make a dustbin whose lid opens automatically."

...eventually became

"Let's build a modular intelligent waste monitoring system with cloud integration, radar-style scanning, geometric fill estimation and a finite state machine."

That's engineering.

Evolution
Semester 4

Version 1
│
├── Automatic Lid
├── Fill Detection
└── Hardware Prototype

        │

        ▼

Version 2
│
├── WiFi
├── Blynk
├── ThingSpeak
├── Twilio
└── Remote Monitoring

        │

        ▼

Version 3
│
├── OOP
├── Three Sensors
├── Rotating Radar
└── Surface Scanning

        │

        ▼

Version 4
│
├── Finite State Machine
├── Intelligent Area Estimation
├── Noise Reduction
└── Better Logic

The Journey

🚀 Version 1

"Can we make the lid open automatically?"

The first working prototype.

Features

Touchless lid opening
Automatic closing
Fill level detection
Overflow prevention

Hardware

ESP32
2 Ultrasonic Sensors
Servo

🌐 Version 2

"Now let's connect it to the Internet."

Added

Blynk Dashboard
ThingSpeak Analytics
SMS Notifications using Twilio
Remote Monitoring

🧠 Version 3

"The code deserves better architecture."

Introduced

Object-Oriented Programming
Sensor abstraction
Three ultrasonic sensors
Rotating scanner
Better modularity

🎯 Version 4

"What if we actually calculate the occupied area instead of guessing?"

Instead of counting obstacle points,

the radar scan now estimates

Unoccupied Area

↓

Occupied Area

↓

Fill Percentage

Additional improvements

Sector averaging
Noise reduction
State machine
Reset mechanism
Better mathematical model

Tech Stack
Hardware
────────────
ESP32
ESP8266
HC-SR04
Servo Motors
Push Buttons
Buzzers

Software
────────────
Arduino IDE
ESP32Servo
ESP8266WiFi
ThingSpeak
Blynk
Twilio API

Simulation
────────────
Wokwi

Future Scope
📷 Camera-based waste classification
🤖 AI-powered waste segregation
📡 LoRaWAN connectivity
🔋 Solar-powered operation
☁️ MQTT-based monitoring
📍 GPS-enabled municipal deployment
📊 Predictive collection scheduling

==============================================
        PROJECT EVOLUTION COMPLETE

Version 1 : ✔
Version 2 : ✔
Version 3 : ✔
Version 4 : ✔

Status:
The dustbin became smarter than expected.

Next Upgrade:
Teach it to roast people who throw garbage beside it.
==============================================
