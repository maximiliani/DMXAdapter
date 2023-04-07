#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <espnowhandler.h>
#include <dmxHandler.h>
#include "settings.h"

/*
* This struct defines the format of the data that is sent over ESP-NOW.
* It consists of a DMX address and a value.
*/
typedef struct dmxData {
    int address;
    int value;
} dmxData;

esp_now_peer_info_t peerInfo;

uint8_t *ownMac = fromMacAddressString(WiFi.macAddress());

/*
* This function checks if the mac address of the sender is different from the mac address of the ESP we are running on.
* This is important because we use our own MAC address as an indicator for a non-existing device.
* Therefore we don't want to include our own MAC address in the list of devices.
*/
bool checkPeer(const uint8_t *mac_addr) {
    for (int i = 0; i < 6; i++) if (mac_addr[i] != ownMac[i]) return false;
    return true;
}

/*
* This function is called when data has been sent.
* It logs the mac address of the sender and the status of the send.
*/
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

/*
* This function is called when data is received over ESP-NOW.
* It checks if the address of the data matches the address of the curtain.
* If so it will call the setRelativePosition function to move the curtain.
*/
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    dmxData tmpData;
    memcpy(&tmpData, incomingData, sizeof(tmpData));
    Serial.print("Bytes received: ");
    Serial.println(len);

    setDMXValue(tmpData.address, tmpData.value);
    Serial.println("Address: " + String(tmpData.address) + " " + String(tmpData.value));

    Serial.println();
}

/*
* This function is called on startup to configure ESPNow to send data.
* It sets the device as a Wi-Fi Station and initializes ESPNow.
* It also loads the broadcast addresses from the settings.
*/
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

/*
* This function is called on startup to configure ESPNow to receive data.
* It sets the device as a Wi-Fi Station and initializes ESPNow by adding the callback function.
*/
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

/*
* This function is called to send data over ESP-NOW.
* It creates a struct with the address and value and sends it.
*/
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