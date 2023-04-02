#ifndef espnowhandler_h
#define espnowhandler_h

#include <Arduino.h>
#include <esp_now.h>

extern uint8_t *ownMac;

void setupESPNOWSend();

void setupESPNOWRecv();

void sendESPNOW(uint16_t address, uint8_t value);

#endif