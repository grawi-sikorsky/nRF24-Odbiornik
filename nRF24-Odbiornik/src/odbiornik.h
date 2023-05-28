#include <Arduino.h>
#include "configuration.h"
#include <ArduinoJson.h>
#include <EEPROM.h>

struct WhistleData
{
  int     getgwizd;
  float   raw;
  float   avg;
};

struct RelaySetting {
  uint8_t relayNumber;
  uint8_t relayType;
  uint16_t relayTime;
  uint16_t relayBlinkTime;
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

    void initializeEEPROM();
    void saveSettings(RelaySetting settings[]);
    void readSettings(RelaySetting settings[]);
    void printRelayEepromSettings();

    void test();



};