#include "configuration.h"

#ifdef DEBUGSERIAL
  int outpin[] = {OUTPIN0, OUTPIN1, OUTPIN2, OUTPIN3, OUTPIN6, OUTPIN7}; // UP OUTPIN4, OUTPIN5
#else
  int outpin[] = {OUTPIN0, OUTPIN1, OUTPIN2, OUTPIN3, OUTPIN4, OUTPIN5, OUTPIN6, OUTPIN7}; // UP OUTPIN4, OUTPIN5
#endif

byte address[][5] = {"Odb0","Odb1","Odb2","Odb3","Odb4","Odb5","Odb6","Odb7"};  // dostepne adresy odbiornikow zgodnie ze zworkami 1-3

const int outpin_array_len = (sizeof(outpin)/sizeof(*outpin));  // DLUGOSC TABLICY PINOW OUTPUT

struct outdata
{
  int     getgwizd;
  float   raw;
  float   avg;
};
outdata nrfdata;

class Odbiornik
{
  private:
    // TIME CZASOMIERZE
    time_t currentTime, prevTime = 0;                             // TIMER WEJSC INPUT - READ_REFRESH_TIME - 100 ms
    time_t outputCurrentTime, prevOutputTime[outpin_array_len];   // TIMER WYJSC OUTPUT - OUTPUT_TIME - 5000 ms
    time_t outputStroboTime[outpin_array_len];                    // TIMER DLA STROBO
    time_t timeout_start_at;                                      // TIMER LICZACY CZAS OD OSTATNIEGO GWIZDNIECIA.

    // nRF24L01 DEFINICJE
    
    int address_nr = 0; // wybor adresu z tablicy powyzej

    bool addr1_State, addr2_State, addr3_State,
    prev_addr1_State, prev_addr2_State, prev_addr3_State;

    bool output_active[outpin_array_len];   // tablica aktywnych pinow do przekaznikow
    bool input_source[outpin_array_len];    // flagi informujace o zrodle zalaczenia wyjscia - jesli true to jest sa to wejscia INPUT lub POMOCNICZY, jesli false to Gwizdek
    bool output_strobo[outpin_array_len];   // flagi informujace o typie/formie zalaczania wyjscia (false=swiatlo stale, true=strobo/wyjscie migajace)
    bool inPin1_State, inPin1_prev_State;
    
    bool gwizd_on    = false;    // info o aktywnym gwizdku

  public:
    bool getActive_output_pin(int i);
    void setActive_output_pin(int i, bool val);
    bool getInput_source(int i);
    void setInput_source(int i, bool val);
    bool getOutput_strobo(int i);


    // Ustawia piny, pinmode,
    void init();

    // inizjalizuje nrfke
    void initRF();

    // SET INFO LED ON or OFF
    void setInfoLED(bool state);

    // SPRAWDZA CZY Z NADAJNIKA DOTARLA WARTOSC TRUE DLA GETGWIZD
    // JESLI TAK WLACZA PRZEKAZNIKI ITP..
    void check_whistle();


    // USTAWIA OUTPUTY W JEDNEJ FUNKCJI
    // PRZYJMUJE NUMER WYJSCIA KTORY CHCEMY AKTYWOWAC [ 0 - 7 ]
    // WIEKSZA WARTOSC ZOSTAJE POMINIETA
    void set_output(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10);
    void set_output_strobo(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10);


    // Sprawdza czy w danych z RF pojawily sie wartosci 11 12 13 21 22 23.
    // Sa to sygnaly z nadajnika pomocniczego [11,12,13 - LEWY] [21,22,23 - PRAWY]
    bool read_input_rf();


    // Sprawdza czy stan wejsc fizycznych w odbiorniku sie zmienil w stosunku do prev_state
    // domyslnie OFF -> HIGH  // ON -> LOW
    // Zwraca TRUE jesli nastapila zmiana
    // Zwraca FALSE jesli nie
    bool read_input_pins();


    // Obsluga wejsc INPUT i przypisanie odpowiednim WYJSCIOM flagi aktywnej
    // Obsluga wejsc z nadajnika pomocniczego RF
    // Tutaj jedynie ustawiamy co ma zostac wlaczone -> czesc wykonawcza w manage_output()
    // konfiguracja:
    // KOSZ LEWY POM && KOSZ LEWY CZER = KOSZ LEWY LED && PODLOGA LEWY LED
    // KOSZ PRAWY POM && KOSZ PRAWY CZER = KOSZ PRAWY LED && PODLOGA PRAWY LED
    void manage_input();

    // OBSLUGA WYJSC BAZUJE NA INFORMACJACH ZDOBYTYCH W MANAGE_INPUT()
    // USTAWIA STANY WYSOKIE I NISKIE W ZALEZNOSCI OD AKTYWNYCH FLAG Z TABLICY outPin_active[]
    void manage_output();

    // POBIERA ADRES ZE ZWOREK I USTAWIA GO DLA RFki
    void setRFaddress();

    // SPRAWDZA CZY NASTAPILA ZMIANA W ZWORKACH
    // JESLI TAK TO USTAWIA NOWY ADRES DLA ODBIORNIKA
    void manage_zworki();
};