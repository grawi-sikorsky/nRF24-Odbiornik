#include "relay.h"
#include "SPI.h"
#include "configuration.h"

void Relay::activate( uint16_t timeMS, uint8_t lightType, uint8_t evoker ){
    if(lightType == Solid){
        this->setActiveTimeMS(timeMS);
        this->setLightType(lightType);
        this->setEvoker(evoker);
        this->isActive = true;
        this->activateTime = millis();
        digitalWriteFast(this->relayPin, LOW);
    }

    #ifdef DEBUGRELAYS
        Serial.println(F("Relay: Activate"));
        Serial.print(F("timeMS: ")); Serial.println(timeMS);
        Serial.print(F("lightType: ")); Serial.println(lightType);
        Serial.print(F("evoker: ")); Serial.println(evoker);
        Serial.print(F("relayPin: ")); Serial.println(relayPin);
        Serial.print(F("activateMillis: ")); Serial.println(this->activateTime);
    #endif
}

void Relay::activate( uint16_t timeMS, uint8_t lightType, uint16_t blinkTime, uint8_t evoker ){
    if(lightType == Blink && isActive == false){
        this->setActiveTimeMS(timeMS);
        this->setLightType(lightType);
        this->setBlinkTimeMS(blinkTime);
        this->setEvoker(evoker);
        this->isActive = true;
        this->activateTime = millis();
        digitalWriteFast(this->relayPin, LOW);
    }
    else if ( isActive == true){
        this->activateTime = millis();
    }
}

void Relay::activate( RelaySetting setting ){
    this->setActiveTimeMS(setting.relayTime);
    this->setLightType(setting.relayType);
    this->setBlinkTimeMS(setting.relayBlinkTime);
    this->setEvoker(setting.relayEvoker);
    this->isActive = true;
    this->activateTime = millis();
    digitalWriteFast(this->relayPin, LOW);
}

void Relay::deactivate()
{
#ifdef DEBUGRELAYS
    Serial.println(F("Deactivate"));
    Serial.print(F("deac millis: "));
    Serial.println(millis());
    Serial.print(F("time elapsed: "));
    Serial.println(millis() - this->activateTime);
    Serial.print(F("relayType: "));
    Serial.println(this->getLightType());
#endif
    this->isActive = false;
    digitalWriteFast(this->relayPin, HIGH);
}

bool Relay::isTimeout(){
    return (millis() - this->activateTime) >= this->activeTimeMS;
}