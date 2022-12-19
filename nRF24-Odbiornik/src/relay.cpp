#include "relay.h"
#include "SPI.h"
#include "configuration.h"

void Relay::activate( int timeMS, int lightType, int evoker ){
    this->setActiveTimeMS(timeMS);
    this->setLightType(lightType);
    this->setEvoker(evoker);
    this->isActive = true;
    this->activateTime = millis();
    digitalWriteFast(this->relayNumber, LOW);

    Serial.println("Relay: Activate");
    Serial.print("timeMS: "); Serial.println(timeMS);
    Serial.print("lightType: "); Serial.println(lightType);
    Serial.print("evoker: "); Serial.println(evoker);
    Serial.print("relayPin: "); Serial.println(relayNumber);
    Serial.print("activateMillis: "); Serial.println(this->activateTime);
}

void Relay::deactivate(){
    Serial.println("Deactivate");
    Serial.print("deac millis: "); Serial.println(millis());
    Serial.print("time elapsed: "); Serial.println(millis() - this->activateTime);
    this->isActive = false;
    digitalWriteFast(this->relayNumber, HIGH);
}

bool Relay::isTimeout(){
    return (millis() - this->activateTime) >= this->activeTimeMS;
}

// void Relay::changePinState(int pinNumber){
//     digitalWriteFast(pinNumber, LOW); // uruchamiamy przekaznik ON
// }