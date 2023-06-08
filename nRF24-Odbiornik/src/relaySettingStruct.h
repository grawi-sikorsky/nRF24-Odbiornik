#ifndef RELAYSETTINGSTRUCT_H
#define RELAYSETTINGSTRUCT_H
#include <Arduino.h>

struct RelaySetting {
  uint16_t relayTime;
  uint16_t relayBlinkTime;
  uint8_t relayEnabled;
  uint8_t relayNumber;
  uint8_t relayType;
  uint8_t relayEvoker;
  uint8_t relayReceiverNumber;
};

#endif