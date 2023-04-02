#ifndef dmxHandler_h
#define dmxHandler_h

#include <Arduino.h>
#include <esp_dmx.h>

extern uint8_t values[DMX_MAX_PACKET_SIZE];
extern byte data[DMX_MAX_PACKET_SIZE];

void setupDMXRead();

void setupDMXWrite();

void readDMX();

void writeDMX();

void setDMXValue(uint16_t address, uint8_t value);

#endif