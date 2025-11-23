/**
 * Kmputer: Bluetooth-Controlled USB HID for M5Stack Cardputer
 *
 * Receives NanoKVM protocol commands via Bluetooth and translates them
 * to USB HID keyboard/mouse events for the target computer.
 *
 * Hardware: M5Stack Cardputer v1.1 (ESP32-S3FN8)
 * License: GNU GPL v3
 * Author: Arthur Endlein
 *
 * USB Identifiers (Development):
 * - VID: 0xFEED (DIY keyboard community)
 * - PID: 0xAE01 (Arthur Endlein initials)
 */

#include <M5Cardputer.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <USBHIDMouse.h>
#include <NimBLEDevice.h>

// USB HID devices
USBHIDKeyboard Keyboard;
USBHIDMouse Mouse;

// BLE server and characteristics
NimBLEServer *pServer = nullptr;
NimBLECharacteristic *pTxCharacteristic = nullptr;
NimBLECharacteristic *pRxCharacteristic = nullptr;

// Connection state
bool deviceConnected = false;
bool oldDeviceConnected = false;

// NanoKVM Protocol constants (from browser/src/libs/device/proto.ts)
#define HEAD1 0x57
#define HEAD2 0xAB
#define ADDR_DEFAULT 0x00

// Command codes
#define CMD_GET_INFO 0x01
#define CMD_SEND_KB_GENERAL_DATA 0x02
#define CMD_SEND_KB_MEDIA_DATA 0x03
#define CMD_SEND_MS_ABS_DATA 0x04
#define CMD_SEND_MS_REL_DATA 0x05
#define CMD_SEND_MY_HID_DATA 0x06

// UUIDs for BLE Serial service (Nordic UART Service compatible)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Display state
uint32_t lastStatusUpdate = 0;
uint32_t commandCount = 0;
uint32_t pairingPin = 0;
bool isPairing = false;

// Forward declarations
void processNanoKVMPacket(uint8_t* packet, size_t len);
void updateDisplay();
void handleKeyboardCommand(uint8_t* data, size_t len);
void handleMouseAbsCommand(uint8_t* data, size_t len);
void handleMouseRelCommand(uint8_t* data, size_t len);

/**
 * BLE Server Callbacks
 */
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE: Client connected");
        M5Cardputer.Display.clear();
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setCursor(10, 10);
        M5Cardputer.Display.println("Connected!");
    };

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE: Client disconnected");
        M5Cardputer.Display.clear();
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setCursor(10, 10);
        M5Cardputer.Display.println("Disconnected");

        // Restart advertising
        delay(500);
        NimBLEDevice::startAdvertising();
        Serial.println("BLE: Advertising restarted");
    }
};

/**
 * BLE Security Callbacks
 */
class SecurityCallbacks : public NimBLESecurityCallbacks {
    uint32_t onPassKeyRequest() {
        Serial.println("BLE: PassKey request");
        return 0;
    }

    void onPassKeyNotify(uint32_t pass_key) {
        Serial.printf("BLE: PassKey notify: %06d\n", pass_key);
        pairingPin = pass_key;
        isPairing = true;

        // Display PIN on screen
        M5Cardputer.Display.clear();
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setCursor(10, 10);
        M5Cardputer.Display.println("Pairing...");
        M5Cardputer.Display.setTextSize(4);
        M5Cardputer.Display.setCursor(10, 50);
        M5Cardputer.Display.printf("%06d", pass_key);
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setCursor(10, 100);
        M5Cardputer.Display.println("Enter this PIN on your");
        M5Cardputer.Display.println("controller device");
    }

    bool onConfirmPIN(uint32_t pin) {
        Serial.printf("BLE: Confirm PIN: %06d\n", pin);

        // Show confirmation prompt
        M5Cardputer.Display.clear();
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setCursor(10, 10);
        M5Cardputer.Display.println("Pairing PIN:");
        M5Cardputer.Display.setTextSize(4);
        M5Cardputer.Display.setCursor(10, 50);
        M5Cardputer.Display.printf("%06d", pin);
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setCursor(10, 100);
        M5Cardputer.Display.println("Press Y to accept");
        M5Cardputer.Display.println("Press N to reject");

        // Wait for user input
        while (true) {
            M5Cardputer.update();
            if (M5Cardputer.Keyboard.isChange()) {
                if (M5Cardputer.Keyboard.isPressed()) {
                    String key = M5Cardputer.Keyboard.keysState().word;
                    if (key == "y" || key == "Y") {
                        isPairing = false;
                        return true;
                    }
                    if (key == "n" || key == "N") {
                        isPairing = false;
                        return false;
                    }
                }
            }
            delay(10);
        }
    }

    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
        Serial.println("BLE: Authentication complete");
        if (desc->sec_state.encrypted) {
            Serial.println("BLE: Connection encrypted");
        }
        isPairing = false;
        updateDisplay();
    }
};

/**
 * BLE Characteristic Callbacks
 */
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();

        if (value.length() > 0) {
            commandCount++;
            processNanoKVMPacket((uint8_t*)value.data(), value.length());
        }
    }
};

void setup() {
    // Initialize M5Cardputer
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    Serial.begin(115200);
    Serial.println("Kmputer: Starting...");

    // Display startup screen
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.println("Kmputer");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 40);
    M5Cardputer.Display.println("Initializing USB HID...");

    // Initialize USB with custom VID/PID
    USB.VID(0xFEED); // DIY keyboard community
    USB.PID(0xAE01); // Arthur Endlein initials
    USB.productName("Kmputer Controller");
    USB.manufacturerName("NanoKVM Project");
    USB.firmwareVersion("1.0.0");
    USB.begin();

    // Initialize HID devices
    Keyboard.begin();
    Mouse.begin();

    delay(500);
    M5Cardputer.Display.println("USB HID ready!");
    M5Cardputer.Display.println("Initializing BLE...");

    // Initialize BLE
    NimBLEDevice::init("Kmputer");

    // Set security
    NimBLEDevice::setSecurityAuth(true, true, true); // bonding, MITM, secure connections
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); // Display + keyboard
    NimBLEDevice::setSecurityCallbacks(new SecurityCallbacks());

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create BLE Service
    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristics
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::NOTIFY
    );

    pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );

    pRxCharacteristic->setCallbacks(new CharacteristicCallbacks());

    // Start service
    pService->start();

    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    NimBLEDevice::startAdvertising();

    M5Cardputer.Display.println("BLE ready!");
    M5Cardputer.Display.println("");
    M5Cardputer.Display.println("Waiting for connection...");
    M5Cardputer.Display.println("Device: Kmputer");

    Serial.println("Kmputer: Ready!");
    Serial.printf("VID:PID = 0x%04X:0x%04X\n", 0xFEED, 0xAE01);
}

void loop() {
    M5Cardputer.update();

    // Handle connection state changes
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        updateDisplay();
    }

    if (!deviceConnected && oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

    // Update display periodically
    if (millis() - lastStatusUpdate > 1000) {
        lastStatusUpdate = millis();
        if (deviceConnected && !isPairing) {
            updateDisplay();
        }
    }

    // Handle emergency disconnect (ESC key)
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        String key = M5Cardputer.Keyboard.keysState().word;
        if (key == "ESC") {
            Serial.println("Emergency disconnect requested");
            if (pServer) {
                pServer->disconnect(pServer->getConnId());
            }
        }
    }

    delay(10);
}

/**
 * Update status display
 */
void updateDisplay() {
    if (isPairing) return; // Don't update during pairing

    M5Cardputer.Display.clear();
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.println("Kmputer");

    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(10, 40);

    if (deviceConnected) {
        M5Cardputer.Display.setTextColor(GREEN);
        M5Cardputer.Display.println("Status: CONNECTED");
        M5Cardputer.Display.printf("Commands: %d\n", commandCount);
        M5Cardputer.Display.printf("Battery: %d%%\n", M5Cardputer.Power.getBatteryLevel());
    } else {
        M5Cardputer.Display.setTextColor(YELLOW);
        M5Cardputer.Display.println("Status: WAITING");
        M5Cardputer.Display.println("Advertising as:");
        M5Cardputer.Display.println("  Kmputer");
    }

    M5Cardputer.Display.setTextColor(DARKGREY);
    M5Cardputer.Display.setCursor(10, 110);
    M5Cardputer.Display.println("Press ESC to disconnect");
}

/**
 * Process NanoKVM protocol packet
 */
void processNanoKVMPacket(uint8_t* packet, size_t len) {
    // Validate minimum packet length
    if (len < 6) {
        Serial.printf("Invalid packet: too short (%d bytes)\n", len);
        return;
    }

    // Validate header
    if (packet[0] != HEAD1 || packet[1] != HEAD2) {
        Serial.printf("Invalid packet: bad header (0x%02X 0x%02X)\n", packet[0], packet[1]);
        return;
    }

    uint8_t cmd = packet[3];
    uint8_t dataLen = packet[4];
    uint8_t* data = &packet[5];

    // Validate data length
    if (len < 6 + dataLen) {
        Serial.printf("Invalid packet: data length mismatch\n");
        return;
    }

    // Process command
    switch (cmd) {
        case CMD_SEND_KB_GENERAL_DATA:
            handleKeyboardCommand(data, dataLen);
            break;

        case CMD_SEND_MS_ABS_DATA:
            handleMouseAbsCommand(data, dataLen);
            break;

        case CMD_SEND_MS_REL_DATA:
            handleMouseRelCommand(data, dataLen);
            break;

        case CMD_GET_INFO:
            Serial.println("CMD: GET_INFO (not implemented)");
            break;

        default:
            Serial.printf("Unknown command: 0x%02X\n", cmd);
            break;
    }
}

/**
 * Handle keyboard command (0x02)
 * Data format: [modifier, 0x00, key1, key2, key3, key4, key5, key6]
 */
void handleKeyboardCommand(uint8_t* data, size_t len) {
    if (len < 8) {
        Serial.println("KB: Invalid data length");
        return;
    }

    uint8_t modifier = data[0];
    uint8_t keys[6] = {data[2], data[3], data[4], data[5], data[6], data[7]};

    // Send keyboard report
    Keyboard.sendReport(modifier, keys, 6);

    Serial.printf("KB: mod=0x%02X keys=[0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X]\n",
                  modifier, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]);
}

/**
 * Handle absolute mouse command (0x04)
 * Data format: [0x02, buttons, x_low, x_high, y_low, y_high, scroll]
 */
void handleMouseAbsCommand(uint8_t* data, size_t len) {
    if (len < 7) {
        Serial.println("MS_ABS: Invalid data length");
        return;
    }

    uint8_t buttons = data[1];
    uint16_t x = data[2] | (data[3] << 8);
    uint16_t y = data[4] | (data[5] << 8);
    int8_t scroll = (int8_t)data[6];

    // Map 0-4095 to screen coordinates (this is approximate)
    // Note: Absolute positioning requires proper coordinate mapping
    Mouse.move(x, y);
    Mouse.click(buttons);
    if (scroll != 0) {
        Mouse.move(0, 0, scroll);
    }

    Serial.printf("MS_ABS: btn=0x%02X x=%d y=%d scroll=%d\n", buttons, x, y, scroll);
}

/**
 * Handle relative mouse command (0x05)
 * Data format: [0x01, buttons, x_delta, y_delta, scroll]
 */
void handleMouseRelCommand(uint8_t* data, size_t len) {
    if (len < 5) {
        Serial.println("MS_REL: Invalid data length");
        return;
    }

    uint8_t buttons = data[1];
    int8_t dx = (int8_t)data[2];
    int8_t dy = (int8_t)data[3];
    int8_t scroll = (int8_t)data[4];

    // Send relative mouse movement
    Mouse.move(dx, dy, scroll, buttons);

    Serial.printf("MS_REL: btn=0x%02X dx=%d dy=%d scroll=%d\n", buttons, dx, dy, scroll);
}
