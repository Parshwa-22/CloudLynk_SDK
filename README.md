# CloudLynk SDK: Universal, Secure IoT Device Framework

The CloudLynk SDK is a high-performance C++ library providing **100% feature coverage** for building portable and secure IoT applications. It establishes a robust, bi-directional communication protocol suitable for production environments across diverse hardware platforms.

## Architectural Design and Principles

The SDK is built on a layered architecture to guarantee **portability** and **security-by-design**:

1. **Hardware Abstraction Layer (HAL):** Isolates application code from platform specifics (I/O, system time, storage), ensuring the core logic runs uniformly on **AVR, ESP32, STM32,** and **Linux SBCs**.  
2. **Network Adapter Interface:** Decouples the core logic from the physical connectivity method (Wi-Fi, Ethernet, Cellular), allowing developers to select their transport layer easily.  
3. **IoT Core:** Manages connectivity state, message routing, scheduling, and enforces the security context.  

---

## Key Features

| Category | Feature | Description |
| :--- | :--- | :--- |
| **Data Flow** | **Bi-Directional V-Pins** | Full support for Virtual Pins (V0-V255), complex data synchronization, and event notifications. |
| | **Advanced Parsing** | `IoTParam` system handles all data types, including float, string, binary, and direct **JSON key access** (`param["key"].asInt()`). |
| **Security** | **TLS/SSL Verification** | Structures for implementing secure MQTT over TLS and mandatory server certificate verification (`IoTVerifyTLS`). |
| | **Data Integrity Hooks** | API ready for **HMAC-SHA256** and **AES-CTR** implementation to secure payloads and prevent tampering. |
| **Device Mgmt** | **Secure OTA** | Full API for Over-The-Air firmware updates, managing status, and enforcing cryptographic **signature verification**. |
| | **System Control** | Functions for remote device diagnostics (`IoTPing`), lifecycle management (`IoTReboot`, `IoTFactoryReset`), and provisioning. |

---

## Installation and Setup

### 1. Library Installation

1. **Download:** Obtain the latest ZIP file of the **`CloudLynk_SDK`** repository.  
2. **Install:** In the Arduino IDE, go to **Sketch > Include Library > Add .ZIP Library...**  

### 2. Dependencies

The SDK requires the following external library:

- **PubSubClient** (Install via **Sketch > Include Library > Manage Libraries...**).  

---

## Minimal Usage Example

This structure demonstrates the clean, functional code required for a secure device application.

```cpp
#include <CloudLynk_SDK.h>

// Instantiate the Network Adapter (Using WiFiAdapter for ESP32/ESP8266)
WiFiAdapter netAdapter; 
#define LED_PIN 2

// 1. Define the V-Pin Handler for V1 (App command to Device action)
IOT_WRITE(1) { 
    int state = param.asInt();
    HAL::digitalWriteHW(LED_PIN, state ? HIGH : LOW);
}

// 2. Define the Connection Handler
IOT_CONNECTED() {
    IoT.IoTDebugLog("Device connected to the platform.");
    IoT.IoTSyncAll(); // Request latest widget states
    
    // Start a 5-second timer to publish sensor data
    IoT.IoTTimerSet(5000, [](){
        float temp = 25.5; // Placeholder for sensor read
        IoT.IoTSendSensor(5, String(temp)); 
    });
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    // CORE CONFIGURATION (MUST BE MODIFIED BY THE USER)
    netAdapter.setConfig("Your_WiFi_SSID", "Your_WiFi_Password");
    IoT.IoTBegin(&netAdapter, "unique-device-id", "YOUR_AUTHORIZATION_TOKEN");

    // Attach handler and events
    IoT.IoTAttachVirtual(1, IOT_write_1);
    IoT.onConnected(IOT_connected_cb);
    
    // Connect securely over TLS (port 8883 is standard for secure MQTT)
    // NOTE: Requires external configuration of the CA certificate for full security.
    IoT.IoTConnect("mqtt.yourplatform.com", 8883, true); 
}

void loop() {
    // ESSENTIAL: Keeps connection alive, processes messages, and runs timers
    IoT.IoTRun(); 
}

