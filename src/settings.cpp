// Created by Maximilian Inckmann on 27.10.22

#include "settings.h"
#include "SPIFFS.h"
#include "WiFi.h"

const int capacity = JSON_OBJECT_SIZE(8) + maxAmountOfESPNowPeers * JSON_OBJECT_SIZE(1) + 16;
StaticJsonDocument <capacity> doc;
bool reloadFromDisk = true;

String toMacAddressString(uint8_t mac[6]) {
    String result = "";
    for (int i = 0; i < 6; i++) {
        result += String(mac[i], HEX);
        if (i < 5) result += ":";
    }
    return result;
}

uint8_t *fromMacAddressString(String mac) {
    uint8_t *result = new uint8_t[6];
    for (int i = 0; i < 6; i++) {
        result[i] = (uint8_t) strtol(&mac.substring(i * 3, i * 3 + 2)[0], NULL, 16);
    }
    return result;
}

String readFileFromSPIFFS(const char *name) {
    String result = "";
    File file = SPIFFS.open(name, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return "";
    }
    while (file.available()) {
        result += (char) file.read();
    }
    file.close();
    return result;
}

void writeFileToSPIFFS(const char *name, String content) {
    File file = SPIFFS.open(name, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(content)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}


void setJSON(String serializedJSON, bool reboot) {
    Serial.println("Changing settings to: " + serializedJSON);
    DeserializationError err = deserializeJson(doc, serializedJSON);
    if (err) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.f_str());
    }
    for (int i = 0; i < maxAmountOfESPNowPeers; i++) {
        String mac = doc["broadcastAddresses"][i];
        if (mac.equals("")) doc["broadcastAddresses"][i] = WiFi.macAddress();
    }
    writeFileToSPIFFS("/settings.json", serializedJSON);
    if (reboot) ESP.restart();
}

StaticJsonDocument <capacity> getJSON() {
    if (reloadFromDisk) {
        String raw = readFileFromSPIFFS("/settings.json");
        setJSON(raw);
        reloadFromDisk = false;
    }
    StaticJsonDocument <capacity> tempDoc;
    tempDoc["mode"] = doc["mode"];
    tempDoc["ssid"] = doc["ssid"];
    tempDoc["password"] = doc["password"];
    tempDoc["mqttServer"] = doc["mqttServer"];
    tempDoc["mqttPort"] = doc["mqttPort"];
    tempDoc["mqttTopic"] = doc["mqttTopic"];
    tempDoc["broadcastAddresses"] = doc["broadcastAddresses"];
    return doc;
}

String getJSONString() {
    String result = "";
    serializeJson(getJSON(), result);
    return result;
}

const char *getSSID() {
    return doc["ssid"];
}

const char *getPassword() {
    return doc["password"];
}

MODES getMode() {
    uint mode = doc["mode"];
    return MODES(mode);
}

uint8_t **getBroadcastAddresses() {
    uint8_t **result = new uint8_t *[maxAmountOfESPNowPeers];
    for (int i = 0; i < maxAmountOfESPNowPeers; i++) result[i] = fromMacAddressString(doc["broadcastAddresses"][i]);
    return result;
}

const char *getMQTTTopic() {
    return doc["mqttTopic"];
}

const char *getMQTTServer() {
    return doc["mqttServer"];
}

uint getMQTTPort() {
    return doc["mqttPort"];
}

void setupSettings() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    Serial.println("Listing main directory");

    File root = SPIFFS.open("/");
    if (!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    Serial.println(getJSONString());
}