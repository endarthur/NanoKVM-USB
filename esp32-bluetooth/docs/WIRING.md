# Wiring and Hardware Setup for RelayKVM

## 🎯 Overview

Good news: **There's almost no wiring needed!** Everything connects via USB cables.

## 🔌 Basic Setup (Minimal)

### Components Needed

| Item | Purpose | Cost | Where to Buy |
|------|---------|------|--------------|
| M5Stack Cardputer v1.1 | KVM controller | ~$40 | M5Stack store, AliExpress |
| USB-C cable | Connect Cardputer to target PC | ~$5 | Any electronics store |
| USB HDMI capture card | Video from target to controller | ~$15 | Amazon, AliExpress |
| HDMI cable | Connect target to capture card | ~$5 | Any electronics store |

**Total cost: ~$65**

### Connection Diagram

```
┌─────────────────────┐
│  Controller PC      │
│  (Your Laptop)      │
│                     │
│  [Bluetooth] ────────────────┐
│  [USB-A]                     │ Wireless BLE
│     │                        │ (keyboard/mouse commands)
│     │                        │
│     └─[USB HDMI Capture]     │
│            │                 │
│            │ HDMI            │
└────────────┼─────────────────┘
             │
             │
┌────────────┼─────────────────┐
│            │ HDMI            │
│       [HDMI Out]             │
│                              │
│      Target PC               │
│   (Computer to Control)      │
│                              │
│       [USB-C Port]           │
│            │                 │
│            │ USB-C Cable     │
│            │                 │
│       ┌────┴────┐            │
│       │Cardputer│◄───────────┘
│       │(RelayKVM)│
│       └─────────┘
└──────────────────────────────┘
```

### Setup Steps

1. **Plug Cardputer into Target PC:**
   - Use USB-C cable
   - Cardputer will enumerate as USB keyboard/mouse
   - Target PC powers the Cardputer
   - No drivers needed (HID class device)

2. **Connect Video Capture:**
   - HDMI cable: Target PC → Capture card
   - USB cable: Capture card → Controller PC
   - Target PC sends video out HDMI
   - Controller PC receives via capture card

3. **Pair Bluetooth:**
   - Controller PC Bluetooth → Cardputer
   - Enter PIN shown on Cardputer screen
   - Commands sent wirelessly

## 📊 Physical Layout

### Desktop Setup

```
Controller Desk:
┌────────────────────────────────────┐
│                                    │
│    [Controller Laptop/PC]          │
│          ▼  ▲                      │
│      (BT) (USB)                    │
│          ▼  ▲                      │
│         [Cardputer]                │
│      USB Capture Card              │
│                                    │
└────────────────────────────────────┘

Target Desk (up to 10m away):
┌────────────────────────────────────┐
│                                    │
│       [Target PC]                  │
│        ▼  ▲                        │
│    (HDMI)(USB-C)                   │
│        ▼  ▲                        │
│   Capture  Cardputer               │
│    Card                            │
│                                    │
└────────────────────────────────────┘
```

### Rack-Mount Setup

```
Controller:                   Target Rack:
┌──────────┐                 ┌──────────────┐
│ Laptop   │                 │ Server 1     │
│          │                 │  [HDMI][USB] │
│  [BT]────┼─────────────────┼─>[Card][Put] │
│  [USB]   │    Wireless     │              │
│   ▲      │     ~10m        │ Server 2     │
│   │      │                 │  [HDMI][USB] │
│   │      │                 │              │
│   └─[Capture Card]         │ Server 3     │
│      ▲                     │  [HDMI][USB] │
│      │                     │              │
│      └─────HDMI 10m────────┤              │
│                            │              │
└────────────────────────────┴──────────────┘
```

## 🔧 Advanced Setup (Optional)

### Multiple Targets with KVM Switch

Add a hardware KVM switch to control multiple computers:

```
Controller PC
     │
     │ Bluetooth (single pairing)
     ▼
  Cardputer ──USB──┐
                   │
                ┌──┴──────────┐
                │ USB KVM     │
                │ Switch      │
                │  1  2  3  4 │
                └──┬──┬──┬──┬─┘
                   │  │  │  │
            Target Target Target Target
              PC1   PC2   PC3   PC4
```

**Benefits:**
- Control multiple PCs with one Cardputer
- Physical USB switching (no ground loop issues)
- Press button on KVM to switch targets

**Recommended KVM switches:**
- UGREEN 4-Port USB KVM (~$40)
- ATEN CS22U 2-Port (~$50)
- TESmart 4-Port (~$80, supports 4K video)

### Long-Range Setup (WiFi Relay)

For ranges > 10m, use WiFi instead of Bluetooth:

```
Controller PC ──WiFi──┐
                      │
                  ┌───┴──────┐
                  │ ESP32    │
                  │ WiFi→BLE │
                  │ Bridge   │
                  └───┬──────┘
                      │ Bluetooth
                      ▼
                  Cardputer ──USB──> Target PC
```

**DIY WiFi-BLE Bridge (ESP32):**
- Receives commands via WiFi
- Forwards to Cardputer via Bluetooth
- Extends range to 50+ meters
- ~$5 additional hardware

### Battery-Powered Portable

For portable use, power Cardputer from its internal battery:

```
Portable Controller:          Target:
┌──────────────┐           ┌──────────┐
│ Smartphone   │           │ Desktop  │
│              │           │          │
│  [Bluetooth] ├───────────┤[USB Port]│
│              │ Wireless  │    │     │
│  [Web App]   │           │    │     │
└──────────────┘           │  ┌─┴────┐│
                           │  │Card- ││
Video over WiFi ◄──────────┤  │puter ││
(capture card              │  └──────┘│
 connected to              │          │
 travel router)            └──────────┘
```

**Battery runtime:**
- Active use: ~3.5 hours
- With power saving: ~7 hours
- With display off: ~11 hours

## ⚡ Power Considerations

### Option 1: Target PC Powers Cardputer (Recommended)

**Pros:**
- ✅ No battery needed
- ✅ Unlimited runtime
- ✅ Simpler setup

**Cons:**
- ❌ Target must be on
- ❌ USB cable required

**Power specs:**
- Cardputer draws: ~450mA @ 5V
- USB 2.0 provides: 500mA max
- USB 3.0 provides: 900mA max
- ✅ Well within spec

### Option 2: Battery-Powered Cardputer

**Pros:**
- ✅ Fully wireless
- ✅ Portable
- ✅ Can turn on target via Wake-on-LAN

**Cons:**
- ❌ Limited runtime
- ❌ Need to recharge

**Charging:**
- USB-C port on Cardputer
- 500mA charging current
- ~3.5 hour charge time (0-100%)

### Option 3: External Power Bank

**Pros:**
- ✅ Extended runtime (20+ hours)
- ✅ Fully wireless
- ✅ Hot-swappable

**Cons:**
- ❌ Extra hardware
- ❌ Bulkier setup

**Recommended:**
- Anker PowerCore 10000 (~$25)
- RAVPower 20000mAh (~$35)

## 🔒 Ground Loop Isolation

### Are Ground Loops a Problem?

**Short answer: Usually no.**

**Why Cardputer helps:**
- ESP32-S3 has internal voltage regulators
- USB isolates different ground domains
- Bluetooth is wireless (no ground connection)

**When you might have issues:**
- Multiple HDMI connections (video ground loops)
- Long cable runs (> 5m)
- Industrial environments (noisy power)

### Solution: USB Isolator (If Needed)

```
Target PC ──USB──> [USB Isolator] ──USB──> Cardputer
                      (~$15)
```

**Recommended isolators:**
- ADUM4160 USB Isolator (~$15)
- Adafruit USB Isolator (~$20)
- UGREEN USB Isolator (~$12)

**Benefits:**
- 2500V galvanic isolation
- Prevents ground loops
- Protects against voltage spikes

**When to use:**
- Industrial control systems
- Medical equipment
- High-EMI environments
- Paranoid security setups

## 🎨 Cable Management

### Clean Desktop Setup

```
┌─────────────────────────────┐
│ Controller PC               │
│ ┌─────────────────────────┐ │
│ │                         │ │
│ │                         │ │
│ │                         │ │
│ └─────────────────────────┘ │
│   │  │                      │
│   ▼  ▼                      │
│  [BT][USB Capture]          │
│   ▲   ▲                     │
│   │   └─────HDMI 3m─────────┤
│   │                         │
│   └──Cardputer (on desk)    │
│      with USB cable         │
│      to target under desk   │
└─────────────────────────────┘
```

**Tips:**
- Use cable ties/clips
- Route HDMI along desk edge
- Keep Cardputer visible (for status display)
- Use short USB-C cable (< 1m) to target

### Portable Kit

**Recommended case:**
- M5Stack hard case
- Pelican 1200 case (~$25)
- Custom 3D printed case

**Contents:**
- Cardputer
- USB-C cable (30cm)
- USB HDMI capture card
- HDMI cable (1m)
- USB-A to USB-C adapter
- Charging cable

## 🛡️ Physical Security

### Securing Cardputer

For production deployments:

1. **Cable Lock:**
   - Attach Cardputer to desk
   - Kensington-style lock (~$20)
   - Prevents theft

2. **Tamper Detection:**
   ```cpp
   // Use IMU to detect if Cardputer is moved
   void checkTamper() {
     float ax, ay, az;
     M5.Imu.getAccel(&ax, &ay, &az);
     if (abs(ax) > 2.0 || abs(ay) > 2.0) {
       // Movement detected!
       disconnectAndAlert();
     }
   }
   ```

3. **Secure Mounting:**
   - 3D print desk mount
   - Cable management clip
   - Adhesive velcro

## 📐 3D Printable Accessories

### Desk Stand for Cardputer

STL files available (TODO):
- Adjustable angle stand
- Cable management clips
- Wall mount bracket

### Multi-Device Rack

For controlling multiple targets:
- Holds 4x Cardputers
- USB hub integration
- Shared capture card

## ✅ Quick Setup Checklist

Before first use:

- [ ] Cardputer firmware flashed (RelayKVM)
- [ ] USB-C cable connected (Cardputer → Target)
- [ ] HDMI cable connected (Target → Capture card)
- [ ] USB capture card connected (Capture → Controller)
- [ ] Bluetooth paired (Controller → Cardputer)
- [ ] Video feed working (check capture software)
- [ ] Test keyboard input (type in target)
- [ ] Test mouse input (move cursor on target)
- [ ] Battery charged (if using wireless)
- [ ] Emergency disconnect tested (ESC key)

## 🚀 Optional: Custom PCB Adapter

For production deployments, create a custom PCB:

**Features:**
- USB-C input (from target)
- USB-C output (to Cardputer)
- Built-in USB isolator
- Power indicator LED
- Emergency disconnect button
- ESD protection
- KVM switch control pins

**Cost:** ~$10 per unit (PCB + components)
**Design:** Available on request

## 📊 Cable Specifications

| Cable | Type | Length | Bandwidth | Notes |
|-------|------|--------|-----------|-------|
| USB-C to Target | USB 2.0 | 0.5-1m | 480 Mbps | HID doesn't need USB 3 |
| HDMI | HDMI 2.0 | 1-3m | 18 Gbps | For 1080p@60Hz |
| USB Capture | USB 3.0 | 0.5-1m | 5 Gbps | Faster = better video |
| Bluetooth | - | ~10m | 1 Mbps | No cable! |

## Summary

**Minimal wiring:**
- 1× USB-C cable (Cardputer → Target)
- 1× HDMI cable (Target → Capture)
- 1× USB cable (Capture → Controller)

**Total wiring time:** < 5 minutes

**No soldering, no breadboards, no complicated connections!**

Would you like me to create actual wiring diagrams in SVG/PNG format, or 3D printable enclosure designs?
