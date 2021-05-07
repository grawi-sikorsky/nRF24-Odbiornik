#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>
#include "odbiornik.h"

Odbiornik odbiornik;
RF24 radio(9, 10); // CE, CSN
outdata nrfdata;

time_t led_time;                                                // TIMER DLA LED
time_t currentTime, prevTime = 0;                               // TIMER WEJSC INPUT - READ_REFRESH_TIME - 100 ms

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

  odbiornik.init();

  odbiornik.manage_zworki();

  odbiornik.initRF();

  #ifdef DEBUGSERIAL
    Serial.println("SETUP:OK \n");
  #endif
}

// LOOP
void loop() {
  // 1. SPRAWDZ TRANSMISJE RADIOWA OD GWIZDKA:
  if (radio.available())                                        // jesli dane sa dostepne ->
  {
    odbiornik.setInfoLED(true);                                    // INFO LED ON
    radio.read(&nrfdata, sizeof(nrfdata));                    // pobierz dane z nadajnika

    #ifdef DEBUGSERIAL
      Serial.print("AVAIL GwizdON: "); Serial.println(nrfdata.getgwizd);
    #endif

    odbiornik.manage_input();    // caly input ogarniemy w jednej funkcji
    led_time = millis();
  }
  else                                                          // jesli danych z nRF24 brak ->
  {
    if(millis() - led_time >= INFO_LED_TIME) odbiornik.setInfoLED(false); //TIMEOUT DLA INFO LEDA
  }

  // 2. SPRAWDZ POZOSTALE WEJSCIA (FIZYCZNE I NAD. POMOCNICZY)
  // OBSLUGA POZOSTALYCH PERYFERIOW
  currentTime = millis();                                       // pobierz czas do odliczania interwalow sprawdzania WEJSC

  if(currentTime - prevTime >= READ_REFRESH_TIME )              // jesli minelo [READ_REFRESH_TIME] ->
  {
    prevTime = currentTime;
    odbiornik.manage_input();                                             // zarzadzaj wejsciami
    odbiornik.manage_output();                                            // zarzadzaj wyjsciami
    odbiornik.manage_zworki();                                            // zarzadzaj zworkami adresowymi
  }

  #ifdef DEBUGSERIAL
    if(currentTime - prev_debug_time >= SERIAL_DEBUG_FREQ )     // jesli minelo [SERIAL_DEBUG_FREQ] ->
    {
      prev_debug_time = currentTime;
      //debug_print_output();
      odbiornik.setInfoLED(!odbiornik.getInfoLED());
      //digitalWriteFast(LEDPIN, !digitalReadFast(LEDPIN));
    }
  #endif
}
