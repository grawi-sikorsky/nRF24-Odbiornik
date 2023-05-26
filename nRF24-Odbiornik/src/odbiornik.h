#include "configuration.h"

static byte address[][5] = {"Odb0","Odb1","Odb2","Odb3","Odb4","Odb5","Odb6","Odb7"};  // dostepne adresy odbiornikow zgodnie ze zworkami 1-3


struct WhistleData
{
  int     getgwizd;
  float   raw;
  float   avg;
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
};