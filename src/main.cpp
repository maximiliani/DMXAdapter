#include <Arduino.h>
#include <ArduinoOTA.h>
#include <dmxHandler.h>
#include <espnowhandler.h>
#include <settings.h>
#include <webserver.h>

dmx_port_t dmxPort = 1;

/*
* This method is called on startup to initialize the device.
* It loads the settings from SPIFFS, starts the webserver and configures everything else.
*/
void setup() {
    Serial.begin(115200);
    setupSettings();
    setupWebserver();

    switch (getMode()) {
        case DMX2ESPNOW: // read DMX and send via espnow
            setupDMXRead();
            setupESPNOWSend();
            break;
        case ESPNOW2DMX: // receive espnow and write to DMX
            setupDMXWrite();
            setupESPNOWRecv();
            break;
        case DMX2MQTT: // read DMX and send via MQTT (not implemented yet)
            setupDMXRead();
            // setupMQTT();
            break;
        case MQTT2DMX: // receive MQTT and write to DMX (not implemented yet)
            setupDMXWrite();
            // setupMQTT();
            break;
    }

    // setup OTA updates
    String OTA_id = "DMXAdapter - " + String(WiFi.macAddress());
    ArduinoOTA.setHostname(OTA_id.c_str());
    ArduinoOTA.begin();
}

void loop() {
    if (getMode() == DMX2ESPNOW || getMode() == DMX2MQTT) readDMX();
    else writeDMX();
    ArduinoOTA.handle();
}