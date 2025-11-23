# Kmputer: Bluetooth-Controlled USB HID for ESP32

ESP32-S3 firmware for M5Stack Cardputer that enables wireless KVM control via Bluetooth, replacing the direct USB connection with a Bluetooth link.

## 🎯 Project Goals

Replace the NanoKVM-USB's direct serial connection with a Bluetooth connection, allowing:
- **Wireless control** of keyboard/mouse over Bluetooth
- **USB HID emulation** on the target computer
- **Secure pairing** using Cardputer's display and keyboard
- **Separate video capture** via cheap USB HDMI capture card

## 🏗️ Architecture

```
Controller PC → Bluetooth → Cardputer (ESP32-S3) → USB HID → Target PC
Controller PC ← USB HDMI Capture Card ← Target PC (video)
```

## 🔌 USB Identifiers

**Current (Development):**
- VID: `0xFEED` (DIY keyboard community standard)
- PID: `0xAE01` (Arthur Endlein initials)
- Product: "Kmputer Controller"
- Manufacturer: "NanoKVM Project"

**Future (Production):**
- VID: `0x1209` (pid.codes for open source projects)
- PID: TBD (will be allocated after successful testing)

## 🔐 Security Features

- **BLE Secure Simple Pairing (SSP)** with authenticated pairing
- **PIN display** on Cardputer screen (6-digit code)
- **Physical confirmation** using Cardputer keyboard
- **Encrypted connection** using Bluetooth 5 security
- **MAC whitelist** (optional)

## 🛠️ Hardware Requirements

- **M5Stack Cardputer v1.1** (ESP32-S3FN8)
- **USB HDMI Capture Card** (~$15)
- **Target computer** with USB port
- **Controller computer** with Bluetooth

## 📋 Features

- [x] USB HID keyboard emulation (6-key rollover)
- [x] USB HID mouse emulation (absolute & relative modes)
- [x] BLE Serial Profile for command reception
- [x] Secure pairing with PIN display
- [x] NanoKVM protocol parser
- [ ] Web interface support (modify browser app)
- [ ] Battery monitoring
- [ ] Connection status display
- [ ] Firmware OTA updates

## 📦 Directory Structure

```
esp32-bluetooth/
├── firmware/           # Arduino/ESP-IDF firmware
│   ├── NanoKVM_BT/    # Main Arduino sketch
│   ├── lib/           # Libraries (protocol, HID, display)
│   └── platformio.ini # PlatformIO config
├── docs/              # Documentation
│   ├── SETUP.md       # Setup instructions
│   ├── PROTOCOL.md    # Protocol documentation
│   └── SECURITY.md    # Security considerations
└── examples/          # Example configurations
```

## 🚀 Quick Start

### 1. Flash Firmware
```bash
cd firmware/NanoKVM_BT
pio run -t upload
```

### 2. Pair Device
1. Power on Cardputer
2. Enable Bluetooth on controller PC
3. Look for "Kmputer" device
4. Enter PIN shown on Cardputer screen

### 3. Modify Browser App
Replace Web Serial API with Web Bluetooth API:
```typescript
const device = await navigator.bluetooth.requestDevice({
  filters: [{ name: 'Kmputer' }],
  optionalServices: ['serial_port_service']
});
```

## 📄 License

This project is part of NanoKVM-USB and licensed under **GNU GPL v3**.

## 🎯 pid.codes Submission Plan

Once tested and stable, we'll submit for official PID allocation:

1. **Requirements Met:**
   - ✅ Open source license (GPL v3)
   - ✅ Hosted on GitHub
   - ✅ Hardware + software both open source

2. **Submission Info:**
   - Project: Kmputer Controller
   - Description: Bluetooth-controlled USB HID KVM device for ESP32-S3
   - Repository: https://github.com/endarthur/NanoKVM-USB
   - License: GNU GPL v3

3. **Target VID/PID:**
   - VID: `0x1209` (pid.codes)
   - PID: Will be assigned by pid.codes team

## 🤝 Contributing

This is an experimental feature for NanoKVM-USB. Contributions welcome!

## 📚 References

- [NanoKVM-USB Protocol](../browser/src/libs/device/proto.ts)
- [ESP32-S3 USB Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/usb.html)
- [pid.codes](https://pid.codes/)
- [M5Stack Cardputer](https://docs.m5stack.com/en/core/Cardputer%20V1.1)

---

**Author:** Arthur Endlein
**Status:** 🚧 Work in Progress
