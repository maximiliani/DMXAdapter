#include <Arduino.h>
#include <esp_dmx.h>
#include <dmxHandler.h>
#include <espNowHandler.h>
#include <settings.h>

QueueHandle_t queue;
unsigned int timer = 0;
bool dmxIsConnected = false;
byte data[DMX_MAX_PACKET_SIZE];
uint8_t values[DMX_MAX_PACKET_SIZE];

/*
* This method configures the DMX driver to read DMX data.
*/
void setupDMXRead() {
    for (int i = 0; i < DMX_MAX_PACKET_SIZE; i++) values[i] = 0;
    dmx_config_t dmxConfig = DMX_DEFAULT_CONFIG;
    dmx_param_config(dmxPort, &dmxConfig);
    dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);

    int queueSize = 1;
    int interruptPriority = 1;
    dmx_driver_install(dmxPort, DMX_MAX_PACKET_SIZE, queueSize, &queue, interruptPriority);
}

/*
* This method configures the DMX driver to write DMX data.
*/
void setupDMXWrite() {
    for (int i = 0; i < DMX_MAX_PACKET_SIZE; i++) values[i] = 0;
    dmx_config_t dmxConfig = DMX_DEFAULT_CONFIG;
    dmx_param_config(dmxPort, &dmxConfig);
    dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
    int queueSize = 0;
    int interruptPriority = 1;
    dmx_driver_install(dmxPort, DMX_MAX_PACKET_SIZE, queueSize, NULL,
                       interruptPriority);
    dmx_set_mode(dmxPort, DMX_MODE_TX);
}

/*
* This method reads DMX data from the DMX driver.
* It is looped to read the DMX data continuously.
*/
void readDMX() {
    dmx_event_t packet;
    if (xQueueReceive(queue, &packet, DMX_RX_PACKET_TOUT_TICK)) {
        if (packet.status == DMX_OK) {

            /* If this is the first DMX data we've received, lets log it! */
            if (!dmxIsConnected) {
                Serial.println("DMX connected!");
                dmxIsConnected = true;
            }

            /* We can read the DMX data into our buffer and increment our timer. */
            dmx_read_packet(dmxPort, data, packet.size);
            timer += packet.duration;

            /* Print a log message every 1 second (1,000,000 microseconds). */
            if (timer >= 100) {
                for (uint16_t i = 1; i < sizeof(data); i++) {
                    String topic = "dmx/" + String(i);
                    uint8_t value = data[i];
                    // Serial.println("Compare data[i]: " + String(data[i]) + " to values[i] " + String(values[i]));
                    if (value != values[i]) {
                        Serial.println("Received DMX data on CH" + String(i) + ": " + String(value));
                        setDMXValue(i, value);
                        if (getMode() == MODES::DMX2ESPNOW) sendESPNOW(i, value);
                    }
                }
                timer -= 100;
            }

        } else {
            /* Uh-oh! Something went wrong receiving DMX. */
            Serial.println("DMX error!");
        }

    } else if (dmxIsConnected) {
        /* If the DMX timed out, but we were previously connected, we will assume
          that there is no more DMX to read. So we can uninstall the DMX driver. */
        Serial.println("DMX timed out! Uninstalling DMX driver...");
        dmx_driver_delete(dmxPort);
        yield();
//      while (true) yield();
    }
}

/*
* This method writes DMX data from the DMX driver.
* It is looped to write the DMX data continuously.
*/
void writeDMX() {
    byte data[DMX_MAX_PACKET_SIZE];
    /* Increment every byte in our packet to the new increment value. Notice
      we don't increment the zeroeth byte, since that is our DMX start code.
      Then we must write our changes to the DMX packet. */
    for (int i = 1; i < DMX_MAX_PACKET_SIZE; ++i) data[i] = values[i];

    dmx_write_packet(dmxPort, data, DMX_MAX_PACKET_SIZE);

    /* Log a message to the Serial Monitor, decrement the packet counter, and
      increment our increment value. */
    // Serial.printf("Sending DMX values");

    /* Now we can transmit the DMX packet! */
    dmx_tx_packet(dmxPort);

    /* We can do some other work here if we want! */
    // Do other work here...

    /* If we have no more work to do, we will wait until we are done sending our
      DMX packet. */
    dmx_wait_tx_done(dmxPort, DMX_TX_PACKET_TOUT_TICK);
}

/*
* This method sets a DMX value in the values array.
* Only this method should be used to set DMX values.
*/
void setDMXValue(uint16_t address, uint8_t value) {
    Serial.println("Setting DMX value on CH " + String(address) + " to " + String(value));
    if (address < DMX_MAX_PACKET_SIZE && value < 256) {
        data[address] = value;
        values[address] = value;
    }
}