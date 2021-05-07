#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>
#include "odbiornik.h"

Odbiornik asdf;
extern RF24 radio(9, 10); // CE, CSN 

time_t led_time;

#ifdef DEBUGSERIAL
  time_t prev_debug_time;
  #define SERIAL_DEBUG_FREQ 3000
#endif

/*
// DEBUG
void debug_print_output()
{
  Serial.print("nrfdata.getgwizd: "); Serial.println(nrfdata.getgwizd);
  Serial.print("outputCurrenttime: "); Serial.println(outputCurrentTime);
  for (int i = 0; i < outpin_array_len; i++)
  {
    Serial.print("outPin_active[");Serial.print(i); Serial.print("]: "); Serial.println(outPin_active[i]);
    Serial.print("prevOutputTime[");Serial.print(i); Serial.print("]: "); Serial.println(prevOutputTime[i]);
    Serial.print("outPin_input[");Serial.print(i); Serial.print("]: "); Serial.println(outPin_input[i]);
  }
    Serial.print("gwizdON: "); Serial.println(gwizd_on);
    Serial.print("timeout_start_at: "); Serial.println(timeout_start_at);

    //setRFaddress();
    for (int i = 0; i < 5; i++)
    {
      Serial.print((char)address[address_nr][i]);
    }
    Serial.println();
    Serial.println("===================");
}
*/


// SETUP
void setup() 
{
  #ifdef DEBUGSERIAL
    Serial.begin(BAUDRATE);
  #endif

  asdf.init();

  asdf.manage_zworki();

  asdf.initRF();

  #ifdef DEBUGSERIAL
    Serial.println("SETUP:OK \n");
  #endif
}

// LOOP
void loop() {
  // 1. SPRAWDZ TRANSMISJE RADIOWA OD GWIZDKA:
  if (radio.available())                                        // jesli dane sa dostepne ->
  {
    asdf.setInfoLED(true);                                    // INFO LED ON
    radio.read(&nrfdata, sizeof(nrfdata));                    // pobierz dane z nadajnika

    #ifdef DEBUGSERIAL
      Serial.print("AVAIL GwizdON: "); Serial.println(nrfdata.getgwizd);
    #endif

    led_time = millis();
  }
  else                                                          // jesli danych z nRF24 brak ->
  {
    //TIMEOUT DLA INFO LEDA
    if(millis() - led_time >= INFO_LED_TIME)
    {
      asdf.setInfoLED(false);
    }
  }

  asdf.check_whistle();                                         // pomiar czy nastapil wzrost


  // 2. SPRAWDZ POZOSTALE WEJSCIA (FIZYCZNE I NAD. POMOCNICZY)
  // OBSLUGA POZOSTALYCH PERYFERIOW
  currentTime = millis();                                       // pobierz czas do odliczania interwalow sprawdzania WEJSC

  if(currentTime - prevTime >= READ_REFRESH_TIME )              // jesli minelo [READ_REFRESH_TIME] ->
  {
    prevTime = currentTime;
    asdf.manage_input();                                             // zarzadzaj wejsciami
    asdf.manage_output();                                            // zarzadzaj wyjsciami
    asdf.manage_zworki();                                            // zarzadzaj zworkami adresowymi
  }

  #ifdef DEBUGSERIAL
    if(currentTime - prev_debug_time >= SERIAL_DEBUG_FREQ )     // jesli minelo [SERIAL_DEBUG_FREQ] ->
    {
      prev_debug_time = currentTime;
      //debug_print_output();
      digitalWriteFast(LEDPIN, !digitalReadFast(LEDPIN));
    }
  #endif
}
