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
#define DEBUGRELAYS
#define READ_REFRESH_TIME 100   // czestotliwosc ms odswiezania wejsc INPUT
#define INFO_LED_TIME     500   // czas swiecenia leda informacyjnego na pcb

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

#ifdef DEBUGSERIAL
    #define RELAYS_COUNT 6
#else
    #define RELAYS_COUNT 8
#endif

#define EEPROM_INIT_PLACE 1023

// Define default hardcoded values using
// [ number, type, time, blinktime]
#define DEFAULT_RELAY_1 {1, 0, 100, 200}
#define DEFAULT_RELAY_2 {2, 1, 200, 400}
#define DEFAULT_RELAY_3 {3, 0, 300, 600}
#define DEFAULT_RELAY_4 {4, 1, 400, 800}
#define DEFAULT_RELAY_5 {5, 0, 500, 1000}
#define DEFAULT_RELAY_6 {6, 1, 600, 1200}
#define DEFAULT_RELAY_7 {7, 0, 500, 1000}
#define DEFAULT_RELAY_8 {8, 1, 600, 1200}

#ifdef DEBUGSERIAL
    #define DEFAULT_RELAYS [RELAYS_COUNT] { \
    DEFAULT_RELAY_1, \
    DEFAULT_RELAY_2, \
    DEFAULT_RELAY_3, \
    DEFAULT_RELAY_4, \
    DEFAULT_RELAY_5, \
    DEFAULT_RELAY_6  \
    }
#else
    #define DEFAULT_RELAYS { \
    DEFAULT_RELAY_1, \
    DEFAULT_RELAY_2, \
    DEFAULT_RELAY_3, \
    DEFAULT_RELAY_4, \
    DEFAULT_RELAY_5, \
    DEFAULT_RELAY_6, \
    DEFAULT_RELAY_7, \
    DEFAULT_RELAY_8 \
    }
#endif
