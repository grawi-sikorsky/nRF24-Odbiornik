#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>

/********************************
*    USTAWIENIA
********************************/
#define BAUDRATE 115200

// KONFIGURACJA
#define DEBUGSERIAL             // DEBUG SERIAL - usuwa 2 output piny dla RX/TX
#define OUTPUT_TIME 2000        // czas wlaczenia przekaznikow po otrzymaniu sygnalu z INPUT 1-4
#define OUTPUT_GWIZD_TIME 2000  // czas wlaczenia przekaznikow po otrzymaniu GWIZDNIECIA
#define FAILSAFE_TIMEOUT  2000  // wylaczenie przekaznikow na wypadek utraty polaczenia z nrf gwizd
#define READ_REFRESH_TIME 100   // czestotliwosc ms odswiezania wejsc INPUT
#define INFO_LED_TIME     500   // czas swiecenia leda informacyjnego na pcb

// KONFIGURACJA WYJSC:
//#define TEST_MODE                 // TESTMODE TO WERSJA PODSTAWOWA: EWRYFINK FAKING ILUMINEJTED
#define EASY_MODE                 // WESJA PODSTAWOWA LANCUT [ GWIZDEK -> OUT1; POMOCNICZE -> OUT2; Reszta wolna]
#define GWIZD_2S                  // GWIZD = 2s LEDOW - wylacza sie automatycznie, jesli nie zdefiniowane to nadajnik wysyla 1, potem 0 aby wylaczyc ledy, a nastepnie 2 jako brak transmisji.
#define STROBO_FREQ 105           // odstep czasu miedzy mignieciami

/********************************
*  PINY
*********************************/
// USTAWIENIA ADRESU
#define ADDR1  5     // Zworka1 - ZW1
#define ADDR2  6     // Zworka2 - ZW2
#define ADDR3  7     // Zworka3 - ZW3

// MISC
#define NEOPIN  4   // NEOPIXEL
#define LEDPIN  14  // LED
#define BTNPIN  15  // w przyszlosci PC1/ pin 15, obecnie A7/ADC7 pin A7

// PINY WEJSCIOWE
#define INPIN1  8     // kosz prawy CZERWONE

// PINY WYJSCIOWE NA PRZEKAZNIKI
#define OUTPIN0 16    //  kosz lewy
#define OUTPIN1 17    //  kosz prawy
#define OUTPIN2 18    //  podloga lewy
#define OUTPIN3 19    //  podloga prawy
#define OUTPIN4 0     //  wolne/RX
#define OUTPIN5 1     //  wolne/TX
#define OUTPIN6 2     //  wolne
#define OUTPIN7 3     //  wolne