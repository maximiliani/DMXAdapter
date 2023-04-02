#include <Arduino.h>
#include <ArduinoOTA.h>
#include <dmxHandler.h>
#include <espnowhandler.h>
#include <settings.h>
#include <webserver.h>

dmx_port_t dmxPort = 1;

void setup() {
    Serial.begin(115200);
    setupSettings();
    setupWebserver();

    switch (getMode()) {
        case DMX2ESPNOW:
            setupDMXRead();
            setupESPNOWSend();
            break;
        case ESPNOW2DMX:
            setupDMXWrite();
            setupESPNOWRecv();
            break;
        case DMX2MQTT:
            setupDMXRead();
            // setupMQTT();
            break;
        case MQTT2DMX:
            setupDMXWrite();
            // setupMQTT();
            break;
    }

    String OTA_id = "DMXAdapter - " + String(WiFi.macAddress());
    ArduinoOTA.setHostname(OTA_id.c_str());
    ArduinoOTA.begin();
}

void loop() {
    if (getMode() == DMX2ESPNOW || getMode() == DMX2MQTT) readDMX();
    else writeDMX();
}