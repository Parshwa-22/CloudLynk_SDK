#ifndef CLOUDLYNK_SDK_H
#define CLOUDLYNK_SDK_H

#include <Arduino.h>
#include <map>
#include <vector>

// ------------------------ Configuration and Constants ------------------------
#define MAX_VPINS 256
#define MAX_TOPIC_LEN 256
#define MAX_PAYLOAD_LEN 2048
#define SECURE_KEY_SIZE 32 
// Add other constants as needed

// Forward declaration for the global IoTCore instance
class IoTCore;
extern IoTCore IoT; // The main object users call (e.g., IoT.IoTRun())

// ------------------------ User Macros (Simplified API) ------------------------
#define IOT_WRITE(V) void IOT_write_##V(const IoTParam& param)
#define IOT_READ(V) void IOT_read_##V(const IoTParam& param)
#define IOT_CONNECTED() void IOT_connected_cb()
#define IOT_DISCONNECTED() void IOT_disconnected_cb()

// ------------------------ IoTParam (Data Parser) Class ------------------------
class IoTParam {
public:
    String payload;
    std::map<String, String> jsonValues;
    std::vector<uint8_t> byteData;

    IoTParam();
    IoTParam(const String& p);
    IoTParam(const uint8_t* data, size_t len);

    // Data Accessors
    int asInt() const;
    float asFloat() const;
    double asDouble() const;
    bool asBool() const;
    String asString() const;
    const std::vector<uint8_t>& asByteArray() const;
    IoTParam operator[](const String& key) const; // For JSON access: param["key"].asInt()
    size_t length() const;

private:
    void parseJson();
};

// ------------------------ HAL (Hardware Abstraction Layer) Namespace ------------------------
namespace HAL {
    // Universal I/O
    void digitalWriteHW(uint8_t pin, uint8_t val);
    int analogReadHW(uint8_t pin);
    void pwmWriteHW(uint8_t pin, int value);
    
    // System Functions
    void begin();
    void reboot();
    unsigned long millisHW();
    void delayHW(unsigned long ms);

    // Storage and OTA stubs
    void storageWriteString(const char* key, const String& s);
    String storageReadString(const char* key);
    void storageClear();
    bool startOTA(const String& url);
    int getOTAStatus();
}

// ------------------------ Network Adapter Interface ------------------------
// The abstract class for any network module.
class NetworkAdapter {
public:
    virtual bool connect(const char* clientId, const char* server, uint16_t port, const char* user, const char* pass, bool secure = false) = 0;
    virtual bool publish(const char* topic, const char* payload, bool retained = false) = 0;
    virtual bool subscribe(const char* topic) = 0;
    virtual void loop() = 0;
    virtual bool isConnected() = 0;
    virtual void setMessageCallback(void (*cb)(const char* topic, const uint8_t* payload, unsigned int len)) = 0;
};

// ------------------------ Concrete Network Adapters ------------------------
#if defined(ESP32) || defined(ESP8266)
class WiFiAdapter : public NetworkAdapter {
    // Private members are defined in the .cpp file
public:
    void setConfig(const String& ssid, const String& pass);
    bool connect(const char* clientId, const char* server, uint16_t port, const char* user, const char* pass, bool secure) override;
    bool publish(const char* topic, const char* payload, bool retained = false) override;
    bool subscribe(const char* topic) override;
    void loop() override;
    bool isConnected() override;
    void setMessageCallback(void (*cb)(const char* topic, const uint8_t* payload, unsigned int len)) override;
    void IoTVerifyTLS(const char* ca_cert); // Public hook for users to set cert
};
#endif
// Placeholder for AVR/Generic boards (for insecure connections)
class PlaceholderAdapter : public NetworkAdapter {
public:
    bool connect(const char* clientId, const char* server, uint16_t port, const char* user, const char* pass, bool secure) override;
    bool publish(const char* topic, const char* payload, bool retained = false) override;
    bool subscribe(const char* topic) override;
    void loop() override;
    bool isConnected() override;
    void setMessageCallback(void (*cb)(const char* topic, const uint8_t* payload, unsigned int len)) override;
};


// ------------------------ IoTCore Class (The SDK Core) ------------------------
class IoTCore {
public:
    typedef void (*VCallback)(const IoTParam& p);
    typedef void (*TimerCb)();

    // Connectivity & Lifecycle
    void IoTBegin(NetworkAdapter* adapter, const String& device_id, const String& token);
    bool IoTConnect(const char* server, uint16_t port, bool secure = false);
    void IoTRun();
    void IoTDisconnect();
    bool IoTIsConnected();
    void IoTHeartbeat(unsigned long ms);

    // Virtual Pins (I/O)
    void IoTAttachVirtual(int v, VCallback cb);
    void IoTVirtualWrite(int v, const String& val);
    void IoTSendSensor(int v, const String& val);
    void IoTNotify(int v, const String& val);
    void IoTSyncAll();
    void IoTSyncVirtual(int v);

    // System & Security
    void IoTReboot();
    void IoTFactoryReset();
    void IoTPing();
    void IoTDeviceInfo();
    void IoTSetAuthToken(const String& token);
    void IoTVerifyTLS(const String& cert); // Global access to set server cert
    String IoTEncrypt(const String& payload);
    String IoTDecrypt(const String& payload);
    void IoTProvision();
    void IoTSetAuthFromApp(const String& authToken);
    
    // Time & Scheduling
    int IoTTimerSet(unsigned long ms, TimerCb cb);
    int IoTTimerOnce(unsigned long ms, TimerCb cb);
    void IoTTimerStop(int id);

    // Logging
    void IoTSave(const String& key, const String& value);
    String IoTLoad(const String& key);
    void IoTDebugLog(const char* msg);
    void IoTErrorLog(int code, const char* msg);

    // Internal Dispatcher (Public for NetworkAdapter callback)
    void onMessage(const char* topic, const uint8_t* payload, unsigned int len);

    // Event Hook Setters
    void onConnected(void (*cb)());
    void onDisconnected(void (*cb)());
    void setEventCb(void (*cb)(const char*, const IoTParam&));

private:
    // This part is implemented in the .cpp file
    // NetworkAdapter* net;
    // VCallback vcallbacks[MAX_VPINS];
    // TimerEntry timers[16];
    
    void checkHeartbeat();
    void timerPoll();
    void subscribeInternalTopics();
};

#endif // CLOUDLYNK_SDK_H