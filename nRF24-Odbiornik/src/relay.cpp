#include "relay.h"
#include "SPI.h"
#include "configuration.h"

void Relay::activate( int timeMS, int lightType, int evoker ){
    if(lightType == Solid){
        this->setActiveTimeMS(timeMS);
        this->setLightType(lightType);
        this->setEvoker(evoker);
        this->isActive = true;
        this->activateTime = millis();
        digitalWriteFast(this->relayNumber, LOW);
    }

    #ifdef DEBUGSERIAL
        Serial.println(F("Relay: Activate"));
        Serial.print(F("timeMS: ")); Serial.println(timeMS);
        Serial.print(F("lightType: ")); Serial.println(lightType);
        Serial.print(F("evoker: ")); Serial.println(evoker);
        Serial.print(F("relayPin: ")); Serial.println(relayNumber);
        Serial.print(F("activateMillis: ")); Serial.println(this->activateTime);
    #endif
}

void Relay::activate( int timeMS, int lightType, int blinkTime, int evoker ){
    if(lightType == Blink && isActive == false){
        this->setActiveTimeMS(timeMS);
        this->setLightType(lightType);
        this->setBlinkTimeMS(blinkTime);
        this->setEvoker(evoker);
        this->isActive = true;
        this->activateTime = millis();
        digitalWriteFast(this->relayNumber, LOW);
    }
    else if ( isActive == true){
        this->activateTime = millis();
    }
}

void Relay::deactivate(){
    #ifdef DEBUGSERIAL
        Serial.println(F("Deactivate"));
        Serial.print(F("deac millis: ")); Serial.println(millis());
        Serial.print(F("time elapsed: ")); Serial.println(millis() - this->activateTime);
        Serial.print(F("relayType: ")); Serial.println(this->getLightType());
    #endif
    this->isActive = false;
    digitalWriteFast(this->relayNumber, HIGH);
}

bool Relay::isTimeout(){
    return (millis() - this->activateTime) >= this->activeTimeMS;
}