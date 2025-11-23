# Android Phone as Bluetooth HID Device

Instead of buying an M5Stack Cardputer ($40), you can use your existing Android phone as a Bluetooth keyboard/mouse relay. This is the **$0 solution** for wireless KVM control.

## Overview

**How it works:**
```
Controller PC (web browser)
    │
    └──> Android Phone (Bluetooth HID)
             │
             └──> Target PC (receives as USB keyboard/mouse)
```

**Wait, Android phones can't act as USB HID devices!**

Correct! But they **can** act as **Bluetooth HID devices** (since Android 9 Pie). Here's the actual topology:

```
Controller PC (web browser)
    │
    └─ WiFi or Bluetooth ─> Android Phone
                              │
                              └─ Bluetooth HID ─> Target PC
```

Android phone acts as a **HID relay**: receives commands from controller, sends as Bluetooth HID to target.

## Requirements

**Android phone:**
- Android 9.0 (Pie) or newer
- Bluetooth 4.0+ (all modern phones)
- Any Android phone from 2018+ should work

**Target PC:**
- Bluetooth receiver (built-in on laptops, USB dongle for desktops)
- Windows 7+, Linux, or macOS (all support Bluetooth HID)

**Controller PC:**
- Web browser with Web Bluetooth support (Chrome, Edge)
- Or WiFi connection to phone's web server

## Option 1: Use Existing Apps

Several apps already exist that turn Android into Bluetooth keyboard/mouse:

### 1. Bluetooth Keyboard & Mouse (Free)

**Google Play:** [Bluetooth Keyboard & Mouse](https://play.google.com/store/apps/details?id=io.appground.blek)

**Features:**
- ✅ Bluetooth HID keyboard + mouse
- ✅ Works with Windows/Linux/macOS
- ✅ Free (ad-supported)
- ❌ No remote control API (can't control from web browser)

**Use case:** Manual control only
```
1. Install app on Android
2. Pair Android phone with target PC via Bluetooth
3. Open app → start HID mode
4. Type/click on phone → controls target PC

Good for: Simple Bluetooth KVM, no remote control needed
```

---

### 2. WearMouse by Google (⭐ Best Reference!)

**GitHub:** [ginkage/wearmouse](https://github.com/ginkage/wearmouse)

**Features:**
- ✅ **Semi-official Google sample code** (by Googler)
- ✅ Full implementation (keyboard + mouse)
- ✅ Actively maintained
- ✅ Proper HID descriptors, QoS, battery callbacks
- ✅ Works on phones (not just Wear OS)
- ✅ Available on Google Play for testing
- ✅ Clean, modern Kotlin code

**Use case:** Primary reference for custom app development

---

### 3. Kontroller (Open Source)

**GitHub:** [rom1v/kontroller](https://github.com/rom1v/kontroller)
**Updated fork:** [arpruss/Kontroller](https://github.com/raghavk92/Kontroller) (October 2024)

**Features:**
- ✅ Open source (can modify for our needs)
- ✅ Bluetooth HID keyboard
- ✅ Simple codebase (~500 lines Kotlin)
- ❌ No mouse support (keyboard only)
- ❌ No remote control API
- ⚠️ Original abandoned (2019), but has recent fork

**Use case:** Secondary reference (simpler than WearMouse)

---

### 4. Serverless Bluetooth Keyboard & Mouse (Free)

**Google Play:** [Serverless Bluetooth Keyboard & Mouse](https://play.google.com/store/apps/details?id=io.appground.serverless)

**Features:**
- ✅ Bluetooth HID keyboard + mouse
- ✅ Works offline (no internet needed)
- ✅ Configurable shortcuts
- ❌ No remote control API

---

**Limitation of existing apps:**

None of these apps expose a **remote control API** that our web interface can use. They're designed for manual phone interaction, not remote automation.

**For our use case, we need to build a custom app** (or modify Kontroller).

---

## Option 2: Custom Android App

To integrate with our web interface, we need an app that:
1. Acts as Bluetooth HID device to target PC
2. Receives commands from controller PC (via WiFi or BLE)
3. Relays keyboard/mouse input to target PC

### Architecture

**Via WiFi (recommended):**
```
Controller Web Browser ─HTTP WebSocket─> Android App ─Bluetooth HID─> Target PC
```

**Via Bluetooth (dual Bluetooth):**
```
Controller PC ─BLE─> Android App ─Bluetooth Classic HID─> Target PC
                     (needs two Bluetooth radios)
```

Most Android phones support **simultaneous BLE + Bluetooth Classic**, so dual-Bluetooth works!

### Implementation: Android Bluetooth HID API

**Android 9+ provides BluetoothHidDevice API:**

```kotlin
// 1. Register as HID device
val bluetoothHidDevice = bluetoothAdapter.getProfileProxy(
    context,
    object : BluetoothProfile.ServiceListener {
        override fun onServiceConnected(profile: Int, proxy: BluetoothProfile) {
            hidDevice = proxy as BluetoothHidDevice

            // Register HID descriptor
            hidDevice.registerApp(
                sdpRecord,     // HID descriptor (keyboard + mouse)
                null,          // No InQoS
                null,          // No OutQoS
                executor,
                callback
            )
        }
    },
    BluetoothProfile.HID_DEVICE
)

// 2. Send keyboard input
fun sendKeyPress(keyCode: Int) {
    val report = byteArrayOf(
        0x00,              // Modifier (none)
        0x00,              // Reserved
        keyCode.toByte(),  // Key code
        0x00, 0x00, 0x00, 0x00, 0x00  // Up to 6 keys
    )

    hidDevice.sendReport(targetDevice, HID_KEYBOARD_REPORT_ID, report)
}

// 3. Send mouse movement
fun sendMouseMove(dx: Int, dy: Int, buttons: Int) {
    val report = byteArrayOf(
        buttons.toByte(),  // Button state (bit 0=left, 1=right, 2=middle)
        dx.toByte(),       // X movement (-127 to 127)
        dy.toByte(),       // Y movement
        0x00               // Wheel
    )

    hidDevice.sendReport(targetDevice, HID_MOUSE_REPORT_ID, report)
}
```

**HID Descriptor (tells target PC we're keyboard + mouse):**

```kotlin
val HID_DESCRIPTOR = byteArrayOf(
    // Keyboard descriptor
    0x05, 0x01.toByte(),        // Usage Page (Generic Desktop)
    0x09, 0x06,                 // Usage (Keyboard)
    0xa1.toByte(), 0x01,        // Collection (Application)
    0x85.toByte(), 0x01,        //   Report ID (1) - Keyboard

    // Modifier keys (Ctrl, Shift, Alt, etc.)
    0x05, 0x07,                 //   Usage Page (Key Codes)
    0x19, 0xe0.toByte(),        //   Usage Minimum (224)
    0x29, 0xe7.toByte(),        //   Usage Maximum (231)
    0x15, 0x00,                 //   Logical Minimum (0)
    0x25, 0x01,                 //   Logical Maximum (1)
    0x75, 0x01,                 //   Report Size (1)
    0x95.toByte(), 0x08,        //   Report Count (8)
    0x81.toByte(), 0x02,        //   Input (Data, Variable, Absolute)

    // Reserved byte
    0x95.toByte(), 0x01,        //   Report Count (1)
    0x75, 0x08,                 //   Report Size (8)
    0x81.toByte(), 0x01,        //   Input (Constant)

    // Key array (6 keys)
    0x95.toByte(), 0x06,        //   Report Count (6)
    0x75, 0x08,                 //   Report Size (8)
    0x15, 0x00,                 //   Logical Minimum (0)
    0x25, 0x65,                 //   Logical Maximum (101)
    0x05, 0x07,                 //   Usage Page (Key Codes)
    0x19, 0x00,                 //   Usage Minimum (0)
    0x29, 0x65,                 //   Usage Maximum (101)
    0x81.toByte(), 0x00,        //   Input (Data, Array)
    0xc0.toByte(),              // End Collection

    // Mouse descriptor
    0x05, 0x01.toByte(),        // Usage Page (Generic Desktop)
    0x09, 0x02,                 // Usage (Mouse)
    0xa1.toByte(), 0x01,        // Collection (Application)
    0x85.toByte(), 0x02,        //   Report ID (2) - Mouse
    0x09, 0x01,                 //   Usage (Pointer)
    0xa1.toByte(), 0x00,        //   Collection (Physical)

    // Buttons (3 buttons)
    0x05, 0x09,                 //     Usage Page (Buttons)
    0x19, 0x01,                 //     Usage Minimum (1)
    0x29, 0x03,                 //     Usage Maximum (3)
    0x15, 0x00,                 //     Logical Minimum (0)
    0x25, 0x01,                 //     Logical Maximum (1)
    0x95.toByte(), 0x03,        //     Report Count (3)
    0x75, 0x01,                 //     Report Size (1)
    0x81.toByte(), 0x02,        //     Input (Data, Variable, Absolute)

    // Padding (5 bits)
    0x95.toByte(), 0x01,        //     Report Count (1)
    0x75, 0x05,                 //     Report Size (5)
    0x81.toByte(), 0x01,        //     Input (Constant)

    // X/Y movement
    0x05, 0x01.toByte(),        //     Usage Page (Generic Desktop)
    0x09, 0x30,                 //     Usage (X)
    0x09, 0x31,                 //     Usage (Y)
    0x15, 0x81.toByte(),        //     Logical Minimum (-127)
    0x25, 0x7f,                 //     Logical Maximum (127)
    0x75, 0x08,                 //     Report Size (8)
    0x95.toByte(), 0x02,        //     Report Count (2)
    0x81.toByte(), 0x06,        //     Input (Data, Variable, Relative)

    // Wheel
    0x09, 0x38,                 //     Usage (Wheel)
    0x15, 0x81.toByte(),        //     Logical Minimum (-127)
    0x25, 0x7f,                 //     Logical Maximum (127)
    0x75, 0x08,                 //     Report Size (8)
    0x95.toByte(), 0x01,        //     Report Count (1)
    0x81.toByte(), 0x06,        //     Input (Data, Variable, Relative)

    0xc0.toByte(),              //   End Collection
    0xc0.toByte()               // End Collection
)
```

### Web Server for Remote Control

**Expose HTTP API on Android:**

```kotlin
// Simple NanoHTTPD server on Android
class ControlServer : NanoHTTPD("0.0.0.0", 8080) {
    override fun serve(session: IHTTPSession): Response {
        val uri = session.uri
        val params = session.parms

        when (uri) {
            "/keyboard" -> {
                val key = params["key"]?.toInt() ?: return badRequest()
                sendKeyPress(key)
                return newFixedLengthResponse(Response.Status.OK, "text/plain", "OK")
            }

            "/mouse" -> {
                val dx = params["dx"]?.toInt() ?: 0
                val dy = params["dy"]?.toInt() ?: 0
                val buttons = params["buttons"]?.toInt() ?: 0
                sendMouseMove(dx, dy, buttons)
                return newFixedLengthResponse(Response.Status.OK, "text/plain", "OK")
            }

            else -> return newFixedLengthResponse(Response.Status.NOT_FOUND, "text/plain", "Not found")
        }
    }
}
```

**Controller web browser calls Android:**

```javascript
// In NanoKVM web interface
const androidUrl = "http://192.168.1.100:8080";  // Android phone's IP

async function sendKeyPress(keyCode) {
    await fetch(`${androidUrl}/keyboard?key=${keyCode}`);
}

async function sendMouseMove(dx, dy, buttons) {
    await fetch(`${androidUrl}/mouse?dx=${dx}&dy=${dy}&buttons=${buttons}`);
}

// Hook into existing NanoKVM event handlers
document.addEventListener('keydown', (e) => {
    sendKeyPress(e.keyCode);
});

canvas.addEventListener('mousemove', (e) => {
    sendMouseMove(e.movementX, e.movementY, e.buttons);
});
```

---

### Alternative: WebSocket for Lower Latency

```kotlin
// Android: WebSocket server
val server = WebSocketServer(8080)

server.onMessage { message ->
    val cmd = JSON.parse(message)

    when (cmd.type) {
        "keyboard" -> sendKeyPress(cmd.keyCode)
        "mouse" -> sendMouseMove(cmd.dx, cmd.dy, cmd.buttons)
    }
}
```

```javascript
// Controller: WebSocket client
const ws = new WebSocket("ws://192.168.1.100:8080");

function sendKeyPress(keyCode) {
    ws.send(JSON.stringify({
        type: "keyboard",
        keyCode: keyCode
    }));
}
```

**Latency comparison:**
- HTTP polling: ~50-100ms
- WebSocket: ~10-20ms
- Bluetooth LE GATT: ~20-30ms

WebSocket is recommended for best responsiveness.

---

## Implementation Options

### Option A: Modify Kontroller (Easiest)

**Steps:**
1. Fork [Kontroller](https://github.com/rom1v/kontroller)
2. Add mouse support (copy HID descriptor from above)
3. Add HTTP/WebSocket server
4. Modify to accept remote commands

**Pros:**
- ✅ Working Bluetooth HID code already exists
- ✅ Simple Kotlin codebase
- ✅ ~500 LOC to start with

**Cons:**
- ❌ Keyboard only (need to add mouse)
- ❌ No remote control (need to add server)

**Estimated effort:** ~8 hours development

---

### Option B: Build from Scratch (Most Control)

**Stack:**
- Kotlin + Android SDK
- BluetoothHidDevice API (Android 9+)
- NanoHTTPD or ktor for web server
- Material Design 3 for UI

**Features:**
- Bluetooth HID keyboard + mouse
- HTTP/WebSocket API for remote control
- Status display (connected devices, battery, etc.)
- Pairing UI
- NanoKVM protocol compatibility

**Estimated effort:** ~20 hours development

---

### Option C: Capacitor App (Web-Based)

**WARNING:** Bluetooth HID is **NOT** available via Web Bluetooth API.

Capacitor/Cordova **cannot** be used for this project because:
- Web Bluetooth only supports BLE GATT (not HID profile)
- No plugin exists for BluetoothHidDevice
- Would need native module anyway

**Verdict:** Must use native Android (Kotlin/Java)

---

## Pairing Process

**Initial setup:**
```
1. Android app: Enable HID mode
2. Android app: Show pairing code (if using SSP)
3. Target PC: Settings → Bluetooth → Add device
4. Target PC: Select Android phone from list
5. Target PC: Enter pairing code
6. Connected! Android now appears as "Wireless Keyboard" + "Wireless Mouse"
```

**Subsequent connections:**
```
1. Android app: Enable HID mode
2. Automatic connection to paired target PC
   (no re-pairing needed)
```

---

## Tested Devices

**Confirmed working with "Bluetooth Keyboard & Mouse" app:**
- ✅ **Samsung Galaxy S24+** (Android 14+) - Tested, works perfectly
- ✅ Most Android 9+ devices should work (BluetoothHidDevice API available)

**Known issues:**
- ⚠️ Some manufacturers (Nokia, Moto, OnePlus 5T/6) have disabled HID Device profile
- ⚠️ Check compatibility: Settings → Bluetooth → Available profiles should show "HID Device"

**Testing your device:**
1. Install "Bluetooth Keyboard & Mouse" from Play Store
2. Enable HID mode in app
3. Pair with target PC
4. If pairing shows keyboard icon (🎹), it works!

---

## Comparison: Android vs ESP32-S3 Cardputer

| Feature | Android Phone | M5Stack Cardputer |
|---------|---------------|-------------------|
| **Cost** | $0 (already have) | $40 |
| **Battery life** | 6-12 hours (need charging) | 3-6 hours |
| **Portability** | ✅ Already carry phone | ❌ Extra device |
| **Screen** | ✅ Large, colorful | ⚠️ Small (135x240) |
| **Remote control** | ✅ Web API (WiFi) | ✅ Web Bluetooth |
| **Durability** | ⚠️ Fragile (screen) | ✅ Rugged |
| **Cool factor** | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| **Development** | Need custom app | ✅ Firmware ready |
| **Wake functionality** | ❌ BT can't USB wake | ✅ USB can wake |

**Winner for:**
- **Cost:** Android ($0 vs $40)
- **Convenience:** Android (already carrying it)
- **Reliability:** Cardputer (dedicated device, USB wake)
- **Coolness:** Cardputer (dedicated hacker device!)

---

## USB Wake Limitation

**Critical difference:** Android phone connects via **Bluetooth**, not **USB**.

This means:
- ❌ Cannot send USB wake signal (target must be awake to receive BT)
- ✅ Can send Wake-on-LAN (if on same network)

**Cardputer advantage:** Connects via **USB**, so can wake from sleep/hibernate:
```
Cardputer ──USB──> Target PC
    └─> Sends USB remote wakeup signal
    └─> Target wakes from S3/S4 sleep
```

**If you need USB wake functionality, use Cardputer instead of Android.**

---

## Recommended Approach

**Phase 1: Test with existing apps**
1. Download "Bluetooth Keyboard & Mouse" app
2. Pair with target PC
3. Test manual control works
4. Validate concept before building custom app

**Phase 2: Build custom app (if needed)**
1. Fork Kontroller as starting point
2. Add mouse support
3. Add WebSocket server
4. Integrate with web interface

**Phase 3: Polish**
1. Add auto-reconnect
2. Battery optimization
3. Status indicators
4. NanoKVM protocol compatibility

---

## Security Considerations

**Bluetooth pairing:**
- Use SSP (Secure Simple Pairing) with numeric comparison
- Android displays pairing code on screen
- User confirms on both Android and target PC

**HTTP API:**
- Only listen on local network (not exposed to internet)
- Optional: Add API key authentication
- Optional: Use HTTPS with self-signed cert

**Best practice:**
```kotlin
// Only bind to private network interfaces
val server = NanoHTTPD("192.168.1.100", 8080)  // Phone's local IP
// NOT: NanoHTTPD("0.0.0.0", 8080)  // Would expose to all networks
```

---

## Development Roadmap

**If building custom Android app:**

**Week 1: Core functionality**
- [ ] Bluetooth HID registration
- [ ] Keyboard input
- [ ] Mouse input
- [ ] Pairing UI

**Week 2: Remote control**
- [ ] HTTP server
- [ ] WebSocket server
- [ ] NanoKVM protocol parser
- [ ] Web interface integration

**Week 3: Polish**
- [ ] Auto-reconnect
- [ ] Battery optimization
- [ ] Status display
- [ ] Error handling

**Week 4: Testing**
- [ ] Test with Windows/Linux/macOS
- [ ] Latency measurements
- [ ] Battery life testing
- [ ] Edge cases (disconnect, etc.)

---

## Sample App Structure

```
android-app/
├── app/
│   ├── src/
│   │   ├── main/
│   │   │   ├── java/com/example/btkvim/
│   │   │   │   ├── MainActivity.kt              # UI
│   │   │   │   ├── BluetoothHidService.kt       # BT HID device
│   │   │   │   ├── WebSocketServer.kt           # Remote control
│   │   │   │   ├── HidDescriptor.kt             # HID report descriptor
│   │   │   │   ├── KeyboardHandler.kt           # Keyboard input
│   │   │   │   ├── MouseHandler.kt              # Mouse input
│   │   │   │   └── NanoKVMProtocol.kt           # Protocol parser
│   │   │   └── AndroidManifest.xml
│   │   └── res/
│   │       └── layout/
│   │           └── activity_main.xml
│   └── build.gradle.kts
└── README.md
```

**Total size:** ~1500 lines of Kotlin

---

## Next Steps

1. **Test existing apps first** - validate Android BT HID works with your target PC
2. **Decide: modify Kontroller or build from scratch**
3. **Implement WebSocket server** for remote control
4. **Integrate with web interface**

Or just **use M5Stack Cardputer** if you want plug-and-play solution! 😄

---

## See Also

- [VIDEO_MODES.md](VIDEO_MODES.md) - Choose video solution (works with Android)
- [BROWSER_INTEGRATION.md](BROWSER_INTEGRATION.md) - Modify web interface for Android HTTP API
- [SECURITY.md](SECURITY.md) - Bluetooth security best practices
- Kontroller reference: https://github.com/rom1v/kontroller
- Android BluetoothHidDevice docs: https://developer.android.com/reference/android/bluetooth/BluetoothHidDevice
