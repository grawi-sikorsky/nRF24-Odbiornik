#include "relay.h"
#include "SPI.h"
#include "configuration.h"

void Relay::activate( int timeMS, int lightType, int evoker ){
    this->setActiveTimeMS(timeMS);
    this->setLightType(lightType);
    this->setEvoker(evoker);
    this->isActive = true;
    digitalWriteFast(this->relayNumber, HIGH);
}

void Relay::deactivate(){
    this->isActive = false;
    digitalWriteFast(this->relayNumber, LOW);
}

bool Relay::isTimeout(){
    return (millis() - this->activateTime) >= this->activeTimeMS;
}

// void Relay::changePinState(int pinNumber){
//     digitalWriteFast(pinNumber, LOW); // uruchamiamy przekaznik ON
// }