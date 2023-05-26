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
#define READ_REFRESH_TIME 100   // czestotliwosc ms odswiezania wejsc INPUT
#define INFO_LED_TIME     500   // czas swiecenia leda informacyjnego na pcb

// KONFIGURACJA WYJSC:
//#define TEST_MODE                 // TESTMODE TO WERSJA PODSTAWOWA: EWRYFINK FAKING ILUMINEJTED
#define EASY_MODE                 // WESJA PODSTAWOWA LANCUT [ GWIZDEK -> OUT1; POMOCNICZE -> OUT2; Reszta wolna]

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

// PINY WEJSCIOWE
#define INPIN1  8     // wejscie fizyczne

// PINY WYJSCIOWE NA PRZEKAZNIKI
#define OUTPIN0 16    //  solid
#define OUTPIN1 17    //  blink
#define OUTPIN2 18    //  solid
#define OUTPIN3 19    //  blink
#define OUTPIN4 0     //  wolne/RX
#define OUTPIN5 1     //  wolne/TX
#define OUTPIN6 2     //  timer off
#define OUTPIN7 3     //  wolne