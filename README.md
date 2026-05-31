# Smart Cooling System — ESP32 Fan Controller

A web-based fan speed controller built with ESP32 and L298N motor driver.
Accessible via local WiFi hotspot with a responsive dashboard UI.

## Features
- Local web dashboard (SoftAP — no internet needed)
- PWM fan speed control via slider and presets
- Eco mode (caps fan at 70% speed)
- Runtime tracker
- Light and dark mode toggle

## Hardware

| Part | Role |
|------|------|
| ESP32 | Web server + PWM control |
| L298N Motor Driver | Fan speed and direction |
| 2-pin DC Fan | Main cooling fan |
| 12V Battery | Power supply |

## Wiring

### ESP32 → L298N
| ESP32 | L298N |
|-------|-------|
| GPIO33 | ENA (PWM speed) |
| GPIO25 | IN1 |
| GPIO26 | IN2 |
| GND | GND |

### Battery → L298N
| Battery | L298N |
|---------|-------|
| + (12V) | 12V terminal |
| - (GND) | GND terminal |

### Fan → L298N
| Fan | L298N |
|-----|-------|
| Fan + | OUT1 |
| Fan - | OUT2 |

> Note: San Ace 4-wire fans are NOT plug-and-play with ESP32 direct PWM.
> They require an open-drain transistor PWM circuit. Use a 2-pin fan + L298N instead.

## How to Use
1. Flash `smart_cooling_system.ino` to your ESP32 via Arduino IDE
2. Connect to WiFi: **SmartCoolingSystem** / Password: **12345678**
3. Open browser → **http://192.168.4.1**

## Developer
Jireh Robellon — Thesis Prototype v2.0
