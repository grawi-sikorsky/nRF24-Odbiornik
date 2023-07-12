#ifndef PAIRINGDATASTRUCT_H
#define PAIRINGDATASTRUCT_H
#include <Arduino.h>

struct PairingData
{
  uint8_t relayEnabled;
  uint8_t relayNumber;
  uint8_t relayType;
  uint8_t relayEvoker;
  uint8_t relayReceiverNumber;
};

#endif