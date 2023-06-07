#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>

/********************************
*    USTAWIENIA
********************************/
#define BAUDRATE 250000

// KONFIGURACJA
#define DEBUGSERIAL             // DEBUG SERIAL - usuwa 2 output piny dla RX/TX
#define DEBUGRELAYS
#define READ_REFRESH_TIME 50    // interwaly odswiezania managera
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
// [ time, blinkTime, enabled, relayNumber, relayType, evoker]
#define DEFAULT_RELAY_1 {400U, 200U, 1U, 1U, 0U, 0U}
#define DEFAULT_RELAY_2 {500U, 400U, 1U, 2U, 0U, 0U}
#define DEFAULT_RELAY_3 {600U, 600U, 1U, 3U, 0U, 0U}
#define DEFAULT_RELAY_4 {700U, 300U, 1U, 4U, 0U, 0U}
#define DEFAULT_RELAY_5 {800U, 300U, 1U, 5U, 0U, 0U}
#define DEFAULT_RELAY_6 {900U, 300U, 1U, 6U, 0U, 1U}
#define DEFAULT_RELAY_7 {1000U, 300U, 1U, 7U, 0U, 0U}
#define DEFAULT_RELAY_8 {1200U, 300U, 1U, 8U, 0U, 0U}

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
