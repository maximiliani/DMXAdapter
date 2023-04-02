#ifndef settings_h
#define settings_h

#include <Arduino.h>
// #include <AsyncJson.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <esp_dmx.h>

#define transmitPin 17
#define receivePin 16
#define enablePin 5
#define defaultSSID "DMXAP"
#define defaultPassword "password"
#define maxAmountOfESPNowPeers 7
//#define amountOfESPNowPeers 20  // should be the maximum amount, but is only available with additional configuration
extern dmx_port_t dmxPort;

enum MODES {
    DMX2ESPNOW = 0,
    ESPNOW2DMX = 1,
    DMX2MQTT = 2,
    MQTT2DMX = 3
};

void setupSettings();

void setJSON(String serializedJSON, bool reboot = false);

StaticJsonDocument<256> getJSON();

String getJSONString();

const char *getSSID();

const char *getPassword();

MODES getMode();

uint8_t **getBroadcastAddresses();

const char *getMQTTTopic();

const char *getMQTTServer();

uint getMQTTPort();

String toMacAddressString(uint8_t mac[6]);

uint8_t *fromMacAddressString(String mac);

#endif