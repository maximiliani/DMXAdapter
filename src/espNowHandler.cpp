#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <espnowhandler.h>
#include <dmxHandler.h>
#include "settings.h"

typedef struct dmxData {
    int address;
    int value;
} dmxData;

esp_now_peer_info_t peerInfo;

uint8_t *ownMac = fromMacAddressString(WiFi.macAddress());

bool checkPeer(const uint8_t *mac_addr) {
    for (int i = 0; i < 6; i++) if (mac_addr[i] != ownMac[i]) return false;
    return true;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    char macStr[18];
    Serial.print("Packet to: ");
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print(macStr);
    Serial.print(" send status:\t");
    Serial.print(status);
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    dmxData tmpData;
    memcpy(&tmpData, incomingData, sizeof(tmpData));
    Serial.print("Bytes received: ");
    Serial.println(len);

    setDMXValue(tmpData.address, tmpData.value);
    Serial.println("Address: " + String(tmpData.address) + " " + String(tmpData.value));

    Serial.println();
}

void setupESPNOWSend() {
    Serial.println();
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    // Set device as a Wi-Fi Station
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);

    // register peer
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    // register first peer  
    uint8_t **peers = getBroadcastAddresses();
    for (int i = 0; i < maxAmountOfESPNowPeers; i++) {
        memcpy(peerInfo.peer_addr, peers[i], 6);
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add peer");
            return;
        }
    }
}

void setupESPNOWRecv() {
    Serial.println();
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void sendESPNOW(uint16_t address, uint8_t value) {
    dmxData data;
    data.address = address;
    data.value = value;

    Serial.println(sizeof(data));
    esp_err_t result = esp_now_send(0, (uint8_t * ) & data, sizeof(dmxData));

    if (result == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }
}