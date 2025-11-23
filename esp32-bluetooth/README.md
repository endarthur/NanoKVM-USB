# Kmputer: Wireless Bluetooth KVM

Wireless KVM solution with flexible hardware options and multiple video modes, perfect for travel, desk setups, or remote access.

## 🎯 Project Goals

Replace the NanoKVM-USB's direct serial connection with a wireless Bluetooth connection, allowing:
- **Wireless control** of keyboard/mouse over Bluetooth
- **Multiple hardware options**: ESP32-S3 Cardputer ($40) or Android phone ($0)
- **Four video modes**: No video (HID only), USB capture, Miracast, or PeerJS/WebRTC
- **Secure pairing** with Bluetooth SSP
- **Works locally or remotely** with PeerJS support

## 🏗️ Architecture

**Two orthogonal problems:**

### 1. HID Relay (Keyboard/Mouse)

**Option A: ESP32-S3 (M5Stack Cardputer)**
```
Controller PC → Bluetooth → Cardputer → USB HID → Target PC
                                ↑
                        (USB Wake capable)
```

**Option B: Android Phone**
```
Controller PC → WiFi/BT → Android → Bluetooth HID → Target PC
```

### 2. Video Transmission (choose based on situation)

**Mode 1: No Video** - Can see target screen physically
```
Just HID control, no video needed (hardware Synergy/Barrier)
```

**Mode 2: USB HDMI Capture** - Most reliable
```
Target PC → HDMI → Capture Card → USB → Controller PC
```

**Mode 3: Miracast** - Wireless display
```
Target PC ─WiFi Direct/Miracast─> Controller PC → OBS Virtual Camera
```

**Mode 4: PeerJS/WebRTC** - Remote access
```
Target PC ─WebRTC P2P─> Controller PC (works across networks)
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

## 🛠️ Hardware Options

### Minimal Setup ($0-40)

**Option A: Android Phone** (free if you already have one)
- Android 9.0+ phone with Bluetooth
- USB cable for charging

**Option B: M5Stack Cardputer** ($40)
- M5Stack Cardputer v1.1 (ESP32-S3FN8)
- USB-C cable

**Both provide:** Wireless keyboard/mouse control via Bluetooth

### Optional Add-ons

- **USB HDMI Capture Card** ($15) - For Mode 2 video
- **HDMI Dummy Plug** ($5) - For lid-closed operation with Miracast/PeerJS
- **HDMI Cable** - For capture card connection

**Total cost range:** $0 (Android only) to $70 (Cardputer + all accessories)

## 📋 Features

### ESP32-S3 Cardputer (Implemented ✅)

- [x] USB HID keyboard emulation (6-key rollover)
- [x] USB HID mouse emulation (absolute & relative modes)
- [x] BLE UART for command reception
- [x] Secure BLE pairing with PIN display
- [x] NanoKVM protocol parser
- [x] USB Wake functionality (S3/S4 sleep)
- [x] Wake menu on Cardputer screen
- [x] M5Launcher compatibility
- [ ] Wake-on-LAN (S5 shutdown)
- [ ] Auto-type PeerJS URL
- [ ] Firmware OTA updates

### Android App (To Be Implemented)

- [ ] Bluetooth HID device mode
- [ ] WebSocket/HTTP API for remote control
- [ ] NanoKVM protocol compatibility
- [ ] Secure pairing UI
- [ ] Status display (battery, connections)
- [ ] Auto-reconnect
- [ ] Wake-on-LAN support

### Video Modes (Documented)

- [x] Mode 1: No Video (HID only)
- [x] Mode 2: USB HDMI Capture (works with existing interface)
- [x] Mode 3: Miracast (documentation complete)
- [ ] Mode 4: PeerJS/WebRTC (sender/receiver pages to be implemented)

## 📦 Directory Structure

```
esp32-bluetooth/
├── firmware/                   # ESP32-S3 firmware (Cardputer)
│   ├── Kmputer/               # Main Arduino sketch
│   └── platformio.ini         # PlatformIO config
├── android/                    # Android app (to be implemented)
│   └── README.md              # Android development guide
├── web/                        # Web interface modifications (to be implemented)
│   ├── peerjs-sender.html     # PeerJS sender page
│   └── bluetooth-adapter.js   # Web Bluetooth adapter
└── docs/                       # Comprehensive documentation
    ├── README.md              # Project overview
    ├── VIDEO_MODES.md         # Compare all 4 video modes ⭐ Start here!
    ├── ANDROID.md             # Android phone setup
    ├── SETUP.md               # Cardputer setup
    ├── WIRING.md              # Hardware connections
    ├── MIRACAST.md            # Miracast setup guide
    ├── PEERJS.md              # PeerJS/WebRTC guide
    ├── WAKE.md                # USB Wake & WoL guide
    ├── SECURITY.md            # Security best practices
    ├── FEATURES.md            # Feature roadmap
    ├── BROWSER_INTEGRATION.md # Modify web interface
    └── M5LAUNCHER.md          # M5Launcher compatibility
```

## 🚀 Quick Start

### Option A: ESP32-S3 Cardputer (Hardware Solution)

**1. Flash Firmware**
```bash
cd firmware/Kmputer
pio run -t upload
```

**2. Connect to Target PC**
```
1. Plug Cardputer into target PC via USB-C
2. Target recognizes as "Kmputer Controller" keyboard/mouse
3. Press 'W' on Cardputer for Wake menu
```

**3. Connect Controller**
```
1. Open web interface in browser
2. Click "Connect via Bluetooth"
3. Select "Kmputer" device
4. Start controlling target PC!
```

See [SETUP.md](docs/SETUP.md) for detailed instructions.

---

### Option B: Android Phone (Free Solution)

**1. Install Bluetooth HID App**
```
Download "Bluetooth Keyboard & Mouse" from Google Play
(or build custom app - see docs/ANDROID.md)
```

**2. Pair with Target PC**
```
1. Open app on Android
2. Enable HID mode
3. Target PC: Settings → Bluetooth → Pair with phone
4. Confirm pairing code
```

**3. Connect Controller**
```
1. Open web interface
2. Enter Android phone's IP address
3. Start controlling target PC!
```

See [ANDROID.md](docs/ANDROID.md) for detailed instructions.

---

### Choosing a Video Mode

See [VIDEO_MODES.md](docs/VIDEO_MODES.md) for comprehensive comparison, or quick guide:

- **Can see target screen?** → Mode 1: No Video (free!)
- **Need BIOS access?** → Mode 2: USB Capture ($15)
- **Both PCs support Miracast?** → Mode 3: Miracast ($0-5)
- **Remote access needed?** → Mode 4: PeerJS ($0)

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

## 📊 Hardware Comparison: Android vs Cardputer

| Feature | Android Phone | M5Stack Cardputer |
|---------|---------------|-------------------|
| **Cost** | $0 (already own) | $40 |
| **Connection to Target** | Bluetooth HID | USB HID |
| **USB Wake** | ❌ No (Bluetooth only) | ✅ Yes (USB connection) |
| **Battery Life** | 6-12 hours | 3-6 hours |
| **Screen** | Large, colorful | Small (135x240 px) |
| **Portability** | Already carry it | Extra device |
| **Durability** | Fragile | Rugged |
| **Development Status** | Need custom app | ✅ Firmware ready |
| **Cool Factor** | ⭐⭐ | ⭐⭐⭐⭐⭐ |

**Bottom line:**
- **Cheapest:** Android phone ($0)
- **Most convenient:** Android (already carrying it)
- **Most reliable:** Cardputer (dedicated device, USB wake)
- **Coolest:** Cardputer (dedicated hacker device!)

## 🤝 Contributing

This is an experimental feature for NanoKVM-USB. Contributions welcome!

**Priority areas:**
1. Android app development (Kotlin + BluetoothHidDevice API)
2. PeerJS sender/receiver pages
3. Web Bluetooth adapter for browser interface
4. Testing on different hardware platforms
5. Documentation improvements

## 📚 References

- [NanoKVM-USB Protocol](../browser/src/libs/device/proto.ts)
- [ESP32-S3 USB Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/usb.html)
- [Android BluetoothHidDevice](https://developer.android.com/reference/android/bluetooth/BluetoothHidDevice)
- [PeerJS Documentation](https://peerjs.com/docs/)
- [pid.codes](https://pid.codes/)
- [M5Stack Cardputer](https://docs.m5stack.com/en/core/Cardputer%20V1.1)
- [Kontroller (Android BT HID reference)](https://github.com/rom1v/kontroller)

## 💡 Use Cases

**Travel / Backpack:**
- Android phone + PeerJS (no extra hardware!)
- Or Cardputer ($40) for dedicated device

**Desk Setup:**
- No Video mode (can see both screens)
- Hardware Synergy/Barrier replacement

**IT Professional:**
- Cardputer + USB capture ($55)
- Reliable BIOS access

**Remote Server:**
- PeerJS mode (works across networks)
- Android or Cardputer

**Conference Room:**
- Miracast mode (wireless presentation control)
- Android phone (discrete)

---

**Author:** Arthur Endlein
**Status:** 🚧 Work in Progress
**Docs:** 80% complete ✅
**Cardputer firmware:** 90% complete ✅
**Android app:** 0% complete (PRs welcome!)
**PeerJS implementation:** 0% complete (PRs welcome!)
