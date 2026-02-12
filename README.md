# PhantomKey – ESP32 Firmware 🔐

ESP32 firmware for the PhantomKey system.
Provides BLE-based rolling code authentication and relay control
for secure vehicle access.

## Role in System
This firmware runs on an ESP32-WROOM-32U and communicates with the
PhantomKey Android application over BLE.

👉 Android app repository:
https://github.com/danijay03/PhantomKey-Android

## Features (MVP)
- BLE GATT server using NimBLE
- Rolling challenge–response authentication
- HMAC-SHA256 verification
- Relay control for unlock signal
- Non-blocking actuator timing
- Designed for automotive power environments

## BLE GATT Design
| Characteristic | UUID | Property |
|---------------|------|----------|
| Challenge | READ | Server → Client |
| Response | WRITE | Client → Server |
| Status | READ / NOTIFY | Server → Client |

## Hardware
- ESP32-WROOM-32U
- 12V → 5V buck converter
- 3.7V Li-ion battery + boost
- Relay module (opto-isolated)

## Notes
⚠ Flash-sensitive pins (CMD, CLK, SD0–SD3) must not be connected.  
⚠ Upload firmware via USB only (no relays connected).

## Status
Prototype firmware complete and stable.
