#ifndef PAIRINGDATASTRUCT_H
#define PAIRINGDATASTRUCT_H
#include <Arduino.h>

struct PairingData
{
  uint8_t pairingRequested;
  uint8_t pairingApproved;
  uint8_t controllerAddress;
  uint8_t controllerNumber;
  uint8_t receiverAddress;
  uint8_t receiverNumber;
  uint8_t dummy;
};

#endif 