#include "relay.h"
#include "SPI.h"
#include "configuration.h"

void Relay::activate( uint16_t duration, uint8_t lightType, uint8_t evoker ){
    if(lightType == Solid){
        this->setDuration(duration);
        this->setLightType(lightType);
        this->setEvoker(evoker);
        this->isActive = true;
        this->activationTime = millis();
        this->lastBlinkTime = millis();
        digitalWriteFast(this->relayPin, LOW);
    }
    #ifdef DEBUGRELAYS
        Serial.println(F("Relay: Activate"));
        Serial.print(F("duration: ")); Serial.println(duration);
        Serial.print(F("lightType: ")); Serial.println(lightType);
        Serial.print(F("evoker: ")); Serial.println(evoker);
        Serial.print(F("relayPin: ")); Serial.println(relayPin);
        Serial.print(F("activateMillis: ")); Serial.println(this->activateTime);
    #endif
}

void Relay::activate( uint16_t duration, uint8_t lightType, uint16_t blinkTime, uint8_t evoker ){
    if(lightType == Blink && isActive == false){
        this->setDuration(duration);
        this->setLightType(lightType);
        this->setBlinkDuration(blinkTime);
        this->setEvoker(evoker);
        this->isActive = true;
        this->activationTime = millis();
        this->lastBlinkTime = millis();
        digitalWriteFast(this->relayPin, LOW);
    }
    else if ( isActive == true){
        this->activationTime = millis();
    }
}

void Relay::activate( RelaySetting setting ){
    this->setDuration(setting.relayTime);
    this->setLightType(setting.relayType);
    this->setBlinkDuration(setting.relayBlinkTime);
    this->setEvoker(setting.relayEvoker);
    this->isActive = true;
    this->activationTime = millis();
    this->lastBlinkTime = millis();
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
    return (millis() - this->activationTime) >= this->duration;
}