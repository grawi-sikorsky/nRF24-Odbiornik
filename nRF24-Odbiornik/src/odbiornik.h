#include <Arduino.h>
#include "configuration.h"
#include <ArduinoJson.h>


struct WhistleData
{
  int     getgwizd;
  float   raw;
  float   avg;
};

struct RelaySetting {
  uint8_t relayNumber = 0;
  uint8_t relayType = 0;
  uint16_t relayTime = 0;
  uint16_t relayBlinkTime = 0;
};

struct SettingsData
{
  uint8_t relay0type;
  uint8_t relay1type;
  uint8_t relay2type;
  uint8_t relay3type;
  uint8_t relay4type;
  uint8_t relay5type;
  uint8_t relay6type;
  uint8_t relay7type;
  uint16_t relay0time;
  uint16_t relay1time;
  uint16_t relay2time;
  uint16_t relay3time;
  uint16_t relay4time;
  uint16_t relay5time;
  uint16_t relay6time;
  uint16_t relay7time;
  uint16_t relay0blinktime;
  uint16_t relay1blinktime;
  uint16_t relay2blinktime;
  uint16_t relay3blinktime;
  // uint16_t relay4blinktime;
  // uint16_t relay5blinktime;
  // uint16_t relay6blinktime;
  // uint16_t relay7blinktime;
  // uint16_t costam;
};

class Odbiornik
{
  private:

    time_t LEDlastActivationTime;

    int address_nr = 0; // wybor adresu z tablicy powyzej
    bool addr_State[3];//, addr2_State, addr3_State,
    bool prev_addr_State[3];//, prev_addr2_State, prev_addr3_State;
    bool inPin1_State, inPin1_prev_State;

  public:
    // Ustawia piny, pinmode,
    void init();

    // inizjalizuje nrfke
    void initRF();

    // SET INFO LED ON or OFF
    void setLEDstate(bool state);
    bool getLEDstate();
    void manageLed();
    void setLedActive();


    bool isWhistleSignal();

    // Sprawdza czy w danych z RF pojawily sie wartosci 11 12 13 21 22 23.
    bool isHelperSignal();

    // Sprawdza czy stan wejsc fizycznych w odbiorniku sie zmienil
    bool isPhysicalSignal();

    bool isSettingsSignal();

    // Obsluga wejsc radiowych
    void manageInputWireless();

    // Obsluga wejsc fizycznych w odbiorniku
    void manageInputPhysical();

    void manageOutputs();

    // POBIERA ADRES ZE ZWOREK I USTAWIA GO DLA RFki
    void setRFaddress();

    // SPRAWDZA CZY NASTAPILA ZMIANA W ZWORKACH
    // JESLI TAK TO USTAWIA NOWY ADRES DLA ODBIORNIKA
    void manageZworki();

    void test();



};