#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>
#include "odbiornik.h"

Odbiornik odbiornik;
RF24 radio(9, 10); // CE, CSN
WhistleData whistleData;

char incomingData[2048];

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
    Serial.println("SETUP: OK! \n");
  #endif
}

// LOOP
void loop() {
  // 1. SPRAWDZ TRANSMISJE RADIOWA OD GWIZDKA:
  if (radio.available())
  {
    radio.read(&incomingData, sizeof(incomingData));
    radio.read(&whistleData, sizeof(whistleData));
    

    odbiornik.setLEDstate(true);

    #ifdef DEBUGSERIAL
      Serial.print("GwizdOn: "); Serial.println(whistleData.getgwizd);
    #endif

    odbiornik.manageInputWireless();
    odbiornik.setLedActive();
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
