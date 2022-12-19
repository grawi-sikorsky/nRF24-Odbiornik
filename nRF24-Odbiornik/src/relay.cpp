#include "relay.h"
#include "SPI.h"

void Relay::activate( int timeMS, int lightType, int evoker ){
    this->setActiveTimeMS(timeMS);
    this->setLightType(lightType);
    this->setEvoker(evoker);
    this->isActive = true;
}

void Relay::deactivate(){
    this->isActive = false;
}