#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>
#include "odbiornik.h"

Odbiornik odbiornik;
RF24 radio(9, 10); // CE, CSN
uint8_t pipeWhistle = 1;
uint8_t pipeSettings = 2;
WhistleData whistleData;
SettingsData settingsData;

time_t currentTime, prevTime = 0;

#ifdef DEBUGSERIAL
  time_t lastDebugTime;
  #define SERIAL_DEBUG_FREQ 3000
#endif

// SETUP
void setup() 
{
  #ifdef DEBUGSERIAL
    Serial.begin(BAUDRATE);
  #endif

  odbiornik.init();
  odbiornik.manageZworki();
  odbiornik.initRF();

  #ifdef DEBUGSERIAL
    Serial.println(F("SETUP: OK! \n"));
  #endif
}

// LOOP
void loop() {
  // 1. SPRAWDZ TRANSMISJE RADIOWA OD GWIZDKA:
  if (radio.available(&pipeWhistle))
  {
    radio.read(&whistleData, sizeof(whistleData));

    odbiornik.setLEDstate(true);

    #ifdef DEBUGSERIAL
      Serial.print(F("Gwizd: ")); Serial.println(whistleData.getgwizd);
    #endif

    odbiornik.manageInputWireless();
    odbiornik.setLedActive();
  }
  if(radio.available(&pipeSettings)){
    radio.read(&settingsData, sizeof(settingsData));
    odbiornik.test();
  }


  // 2. SPRAWDZ POZOSTALE WEJSCIA (FIZYCZNE)
  currentTime = millis();

  if(currentTime - prevTime >= READ_REFRESH_TIME )
  {
    prevTime = currentTime;
    odbiornik.manageInputPhysical();
    odbiornik.manageOutputs();
    odbiornik.manageZworki();
    odbiornik.manageLed();
    
  }

  #ifdef DEBUGSERIAL
    if(currentTime - lastDebugTime >= SERIAL_DEBUG_FREQ )
    {
      lastDebugTime = currentTime;
      odbiornik.setLEDstate(!odbiornik.getLEDstate());
    }
  #endif

}
