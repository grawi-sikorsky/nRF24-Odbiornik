#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>

/********************************
*    USTAWIENIA
********************************/
//#define CLICKER_MODE            // Clicker mode with H-bridge for controlling electrical actuator
#define RELAYS_MODE             // Relays mode as usual for Lights etc.

#define BAUDRATE 250000

// KONFIGURACJA
// #define DEBUGSERIAL             // DEBUG SERIAL - usuwa 2 output piny dla RX/TX
// #define DEBUGRELAYS
// #define DEBUG_STUB
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

// PINY WYJSCIOWE W GNIEZDZIE
#define OUTPIN0 16    //  ADC
#define OUTPIN1 17    //  ADC
#define OUTPIN2 18    //  ADC
#define OUTPIN3 19    //  ADC
#define OUTPIN4 0     //  wolne/RX
#define OUTPIN5 1     //  wolne/TX
#define OUTPIN6 2     //  
#define OUTPIN7 3     //  

/********************************
*  RELAYS MODE
*********************************/
#ifdef RELAYS_MODE
    #ifdef DEBUGSERIAL
        #define RELAYS_COUNT 6
    #else
        #define RELAYS_COUNT 8
    #endif

    #define EEPROM_INIT_PLACE 1023

    // Define default hardcoded values using
    // relayType: 0 Solid, 1 Blink
    // evoker: 0 Whistle, 1 WhistleButton, 2 Helper, 3 Physical
    // [ time, blinkTime, enabled, relayNumber, relayType, evoker]

    //                      {time,      blink,  ena,    num,    type,   evoker}
    #define DEFAULT_RELAY_1 {2000U,		250U,	1U,		1U,		1U,		0U}
    #define DEFAULT_RELAY_2 {2000U,		250U,	1U,		2U,		1U,		2U}
    #define DEFAULT_RELAY_3 {200U,		1000U,	1U,		3U,		0U,		0U}
    #define DEFAULT_RELAY_4 {300U,		1000U,	1U,		4U,		0U,		1U}
    #define DEFAULT_RELAY_5 {1000U,		1000U,	1U,		5U,		0U,		2U}
    #define DEFAULT_RELAY_6 {1000U,		150U,	1U,		6U,		1U,		1U}
    #define DEFAULT_RELAY_7 {1000U,		300U,	0U,		7U,		0U,		0U}
    #define DEFAULT_RELAY_8 {1200U,		300U,	0U,		8U,		0U,		0U}

    #ifdef DEBUGSERIAL
        #define DEFAULT_RELAYS { \
            DEFAULT_RELAY_1, \
            DEFAULT_RELAY_2, \
            DEFAULT_RELAY_3, \
            DEFAULT_RELAY_4, \
            DEFAULT_RELAY_5, \
            DEFAULT_RELAY_6  \
        }
    #else
        #define DEFAULT_RELAYS [RELAYS_COUNT]{ \
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
#endif
/********************************
*  CLICKER MODE
*********************************/
#ifdef CLICKER_MODE
    #define LEFT    0
    #define RIGHT   1

    // ADC
    #define FORWARD_PIN_A         OUTPIN0
    #define BACKWARD_PIN_A        OUTPIN1
    #define FORWARD_PIN_B         OUTPIN2
    #define BACKWARD_PIN_B        OUTPIN3

    // non-ADC
    #define FORWARD_PIN_C         OUTPIN4
    #define BACKWARD_PIN_C        OUTPIN5
    #define FORWARD_PIN_D         OUTPIN6
    #define BACKWARD_PIN_D        OUTPIN7

    #define OPEN_TIME           1000
    #define CLOSE_TIME          1000
#endif