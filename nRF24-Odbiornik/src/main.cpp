#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>

// PINY WEJSCIOWE
#define INPIN1  5     // kosz lewy POMARANCZOWE
#define INPIN2  6     // kosz lewy CZERWONE
#define INPIN3  7     // kosz prawy POMARANCZOWE
#define INPIN4  8     // kosz prawy CZERWONE

// PINY WYJSCIOWE NA PRZEKAZNIKI
#define OUTPIN1 16    //  kosz lewy
#define OUTPIN2 17    //  kosz prawy
#define OUTPIN3 18    //  podloga lewy
#define OUTPIN4 19    //  podloga prawy
#define OUTPIN5 0     //  wolne/RX
#define OUTPIN6 1     //  wolne/TX
#define OUTPIN7 2     //  wolne
#define OUTPIN8 3     //  wolne
int outpin[] = {OUTPIN1, OUTPIN2, OUTPIN3, OUTPIN4, OUTPIN7, OUTPIN8}; // UP OUTPIN5, OUTPIN6
const int outpin_array_len = (sizeof(outpin)/sizeof(*outpin));

#define NEOPIN  4   // NEOPIXEL
#define LEDPIN  14  // LED
#define BTNPIN  15  // w przyszlosci PC1/ pin 15, obecnie A7/ADC7 pin A7

// KONFIGURACJA
// Zakomentuj OUTPUT_RAZEM  jesli chcesz aby odbiornik mial konfigurowalne wyjscia OUT1 - OUT8
// Zakomentuj INPUT_RAZEM   jeśli chcesz aby odbiornik mial konfigurowalne wejscia IN1 - IN4
#define OUTPUT_RAZEM
#define INPUT_RAZEM

#define OUTPUT_TIME 4000    // czas wlaczenia przekaznikow po otrzymaniu sygnalu z INPUT 1-4

#ifndef OUTPUT_RAZEM
  #define OUTPUT_OSOBNO
#endif
#ifndef INPUT_RAZEM
  #define INPUT_OSOBNO
#endif

// 
#define READ_REFRESH_TIME 100

bool inPin1_State, inPin1_prev_State,
    inPin2_State, inPin2_prev_State,
    inPin3_State, inPin3_prev_State,
    inPin4_State, inPin4_prev_State;

bool outPin_active[outpin_array_len];

// nRF24L01 DEFINICJE
const byte address[17] = "1100110011001100";  // adres nadajnika [todo: zrobic to bezpieczniejsze]
RF24 radio(9, 10); // CE, CSN 
float blow_data;

// TIME
time_t currentTime, prevTime = 0;
time_t outputCurrentTime, prevOutputTime = 0;
//
float bme_data;
int data=0;

// Sprawdza czy stan wejsc sie zmienil w stosunku do prev_state
// domyslnie HIGH -> zmiana: LOW
bool read_input_pins()
{
  inPin1_State = digitalReadFast(INPIN1);
  inPin2_State = digitalReadFast(INPIN2);
  inPin3_State = digitalReadFast(INPIN3);
  inPin4_State = digitalReadFast(INPIN4);

  if( inPin1_State != inPin1_prev_State || 
      inPin2_State != inPin2_prev_State || 
      inPin3_State != inPin3_prev_State || 
      inPin4_State != inPin4_prev_State )
    return true;
  else 
    return false;
}

// Obsluga wejsc LOW HIGH CHANGE i przypisanie odpowiednim WYJSCIOM flagi aktywnej
// Obsluga flag w manage_output()
void manage_input()
{
  if (read_input_pins() == true)
  {
    // PIN 1
    if( inPin1_State != inPin1_prev_State )
    {
      if(inPin1_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 1 zmiana: HIGH -> LOW"));
        outPin_active[0] = true;
        outPin_active[1] = true;
        outPin_active[2] = true;
        outPin_active[3] = true;
        outPin_active[4] = true;
        outPin_active[5] = true;
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 1 zmiana: LOW -> HIGH"));
        outPin_active[0] = false;
        outPin_active[1] = false;
        outPin_active[2] = false;
        outPin_active[3] = false;
        outPin_active[4] = false;
        outPin_active[5] = false;
      }

      inPin1_prev_State = inPin1_State; // przypisz obecna wartosc
    }
    // PIN 2
    if( inPin2_State != inPin2_prev_State )
    {
      if(inPin2_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 2 zmiana: HIGH -> LOW"));
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 2 zmiana: LOW -> HIGH"));
      }

      inPin2_prev_State = inPin2_State; // przypisz obecna wartosc
    }
    // PIN 3
    if( inPin3_State != inPin3_prev_State )
    {
      if(inPin3_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 3 zmiana: HIGH -> LOW"));
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 3 zmiana: LOW -> HIGH"));
      }

      inPin3_prev_State = inPin3_State; // przypisz obecna wartosc
    }
    // PIN 4
    if( inPin4_State != inPin4_prev_State )
    {
      if(inPin4_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 4 zmiana: HIGH -> LOW"));
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 4 zmiana: LOW -> HIGH"));
      }

      inPin4_prev_State = inPin4_State; // przypisz obecna wartosc
    }
  }
}

void manage_output()
{
  outputCurrentTime = millis();

  if(outputCurrentTime - prevOutputTime >= OUTPUT_TIME)
  {
    prevOutputTime = outputCurrentTime;
  }

  for(int i=0; i <= outpin_array_len-1; i++)
  {
    if(outPin_active[i] == true)
    {
      digitalWriteFast(outpin[i], LOW);
      Serial.print("for:out");
      Serial.println(outpin[i]);
    }
  }
}

//void zaswiec(int )

void setup() 
{
  Serial.begin(115200);

  // PINS
  pinModeFast(INPIN1, INPUT_PULLUP);
  pinModeFast(INPIN2, INPUT_PULLUP);
  pinModeFast(INPIN3, INPUT_PULLUP);
  pinModeFast(INPIN4, INPUT_PULLUP);

  pinModeFast(OUTPIN1, OUTPUT);
  pinModeFast(OUTPIN2, OUTPUT);
  pinModeFast(OUTPIN3, OUTPUT);
  pinModeFast(OUTPIN4, OUTPUT);
  //pinModeFast(OUTPIN5, OUTPUT); // RX
  //pinModeFast(OUTPIN6, OUTPUT); // RX
  pinModeFast(OUTPIN7, OUTPUT);
  pinModeFast(OUTPIN8, OUTPUT);

  digitalWriteFast(OUTPIN1, HIGH);
  digitalWriteFast(OUTPIN2, HIGH);
  digitalWriteFast(OUTPIN3, HIGH);
  digitalWriteFast(OUTPIN4, HIGH);
  //digitalWriteFast(OUTPIN5, HIGH);  // RX
  //digitalWriteFast(OUTPIN6, HIGH);  // TX
  digitalWriteFast(OUTPIN7, HIGH);
  digitalWriteFast(OUTPIN8, HIGH);

  inPin1_State = inPin1_prev_State = digitalReadFast(INPIN1);   // zakladamy ze stan bedzie spoczynkowy (!)
  inPin2_State = inPin2_prev_State = digitalReadFast(INPIN2); 
  inPin3_State = inPin3_prev_State = digitalReadFast(INPIN3); 
  inPin4_State = inPin4_prev_State = digitalReadFast(INPIN4); 

  // nRF24L01
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  pinMode(10, OUTPUT);

  Serial.println("SETUP:OK");
}

void loop() {
  // 1. SPRAWDZ TRANSMISJE RADIOWA OD GWIZDKA:
  if (radio.available()) 
  {
    radio.read(&bme_data, sizeof(bme_data));
    Serial.print("Odebrano:");
    Serial.println(bme_data);
    // TODO BLA BLA BLA
  }

  // 2. SPRAWDZ WEJSCIA IN1-IN4:
  currentTime = millis();

  if(currentTime - prevTime >= READ_REFRESH_TIME )
  {
    prevTime = currentTime;
    manage_input();
    manage_output();
  }

  // 3. USTAW WYJSCIA:

}
