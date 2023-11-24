#include "avr8-stub.h"
#include "app_api.h"

#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>
#include "odbiornik.h"

Odbiornik odbiornik;
RF24 radio(9, 10); // CE, CSN
WhistleData whistleData;
RelaySetting relaySetting;

time_t currentTime, prevTime = 0;

#ifdef DEBUGSERIAL
  time_t lastDebugTime;
  #define SERIAL_DEBUG_FREQ 3000
#endif

// SETUP
void setup() 
{
  debug_init();
  
  #ifdef DEBUGSERIAL
    delay(6000); // just to see setup serial info
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

  byte pipe_num = 0;

  if (radio.available(&pipe_num))
  {
    #ifdef DEBUGSERIAL
      Serial.print(F("PipeNumber [available]: ")); Serial.println(pipe_num);
    #endif

    if(pipe_num == EWhistle){
      odbiornik.setLEDstate(true);
      radio.read(&whistleData, sizeof(whistleData));

      #ifdef DEBUGSERIAL
        Serial.print(F("Gwizd: ")); Serial.println(whistleData.command);
      #endif

      //odbiornik.manageInputWireless();
      odbiornik.manageInputWirelessV2();

      odbiornik.setLedActive();
    }
    else if(pipe_num == EController){
      radio.read(&relaySetting, sizeof(relaySetting));
      if(odbiornik.isInSettingsMode()){
        odbiornik.processSettings();
      }
    }
  }


  // 2. SPRAWDZ POZOSTALE WEJSCIA (FIZYCZNE)
  currentTime = millis();

  // if(currentTime - prevTime >= READ_REFRESH_TIME )
  // {
    prevTime = currentTime;
    odbiornik.manageInputPhysical();
    odbiornik.manageOutputs();
    odbiornik.manageZworki();
    odbiornik.manageLed();
  // }

  #ifdef DEBUGSERIAL
    if(currentTime - lastDebugTime >= SERIAL_DEBUG_FREQ )
    {
      lastDebugTime = currentTime;
      odbiornik.setLEDstate(!odbiornik.getLEDstate());
    }
  #endif

}
