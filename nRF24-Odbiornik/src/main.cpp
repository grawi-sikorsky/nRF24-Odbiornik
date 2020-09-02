#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>

#define BAUDRATE 250000

// PINY WEJSCIOWE
#define INPIN1  5     // kosz lewy POMARANCZOWE
#define INPIN2  6     // kosz lewy CZERWONE
#define INPIN3  7     // kosz prawy POMARANCZOWE
#define INPIN4  8     // kosz prawy CZERWONE

// PINY WYJSCIOWE NA PRZEKAZNIKI
#define OUTPIN0 16    //  kosz lewy
#define OUTPIN1 17    //  kosz prawy
#define OUTPIN2 18    //  podloga lewy
#define OUTPIN3 19    //  podloga prawy
#define OUTPIN4 0     //  wolne/RX
#define OUTPIN5 1     //  wolne/TX
#define OUTPIN6 2     //  wolne
#define OUTPIN7 3     //  wolne
int outpin[] = {OUTPIN0, OUTPIN1, OUTPIN2, OUTPIN3, OUTPIN6, OUTPIN7}; // UP OUTPIN4, OUTPIN5
const int outpin_array_len = (sizeof(outpin)/sizeof(*outpin));  // DLUGOSC TABLICY PINOW OUTPUT

#define NEOPIN  4   // NEOPIXEL
#define LEDPIN  14  // LED
#define BTNPIN  15  // w przyszlosci PC1/ pin 15, obecnie A7/ADC7 pin A7

// KONFIGURACJA
// Zakomentuj OUTPUT_RAZEM  jesli chcesz aby odbiornik mial konfigurowalne wyjscia OUT1 - OUT8
// Zakomentuj INPUT_RAZEM   jeśli chcesz aby odbiornik mial konfigurowalne wejscia IN1 - IN4
#define OUTPUT_RAZEM
#define INPUT_RAZEM

#define OUTPUT_TIME 8000    // czas wlaczenia przekaznikow po otrzymaniu sygnalu z INPUT 1-4

#ifndef OUTPUT_RAZEM
  #define OUTPUT_OSOBNO
#endif
#ifndef INPUT_RAZEM
  #define INPUT_OSOBNO
#endif

#define READ_REFRESH_TIME 100
#define RF_OFF_TIME 5000

bool inPin1_State, inPin1_prev_State,
    inPin2_State, inPin2_prev_State,
    inPin3_State, inPin3_prev_State,
    inPin4_State, inPin4_prev_State;

bool outPin_active[outpin_array_len];

// nRF24L01 DEFINICJE
const byte address[17] = "1100110011001100";  // adres nadajnika [todo: zrobic to bezpieczniejsze]
RF24 radio(9, 10); // CE, CSN 

// TIME
time_t currentTime, prevTime = 0;                             // TIMER WEJSC INPUT - READ_REFRESH_TIME - 100 ms
time_t outputCurrentTime, prevOutputTime[outpin_array_len];   // TIMER WYJSC OUTPUT - OUTPUT_TIME - 8000 ms
time_t rfoffTime;                                             // TIMER LICZACY CZAS OD OSTATNIEGO ODBIORU DANYCH Z nRF (potrzebny m.in do pozniejszego rozbiegu sredniej)
// PRESSURE DEFINICJE
float bme_data;             // dane RAW prosto z nadajnika
float bme_tbl[100];        // dane RAW usrednione do 100 probek
float bme_avg = 0;
int   bme_avg_i = 0;
bool  bme_rozbieg = true;
#define BME_AVG_COUNT 25


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
// konfiguracja:
// KOSZ LEWY POM && KOSZ LEWY CZER = KOSZ LEWY LED && PODLOGA LEWY LED
// KOSZ PRAWY POM && KOSZ PRAWY CZER = KOSZ PRAWY LED && PODLOGA PRAWY LED
void manage_input()
{
  if (read_input_pins() == true)
  {
    // PIN 1 KOSZ LEWY POM
    if( inPin1_State != inPin1_prev_State )
    {
      if(inPin1_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 1 zmiana: HIGH -> LOW"));
        outPin_active[0] = true;
        outPin_active[2] = true;
        prevOutputTime[0] = millis();
        prevOutputTime[2] = millis();
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 1 zmiana: LOW -> HIGH"));
      }

      inPin1_prev_State = inPin1_State; // przypisz obecna wartosc
    }
    // PIN 2 KOSZ LEWY CZER
    if( inPin2_State != inPin2_prev_State )
    {
      if(inPin2_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 2 zmiana: HIGH -> LOW"));
        outPin_active[0] = true;
        outPin_active[2] = true;
        prevOutputTime[0] = millis();
        prevOutputTime[2] = millis();
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 2 zmiana: LOW -> HIGH"));
      }

      inPin2_prev_State = inPin2_State; // przypisz obecna wartosc
    }
    // PIN 3 KOSZ PRAWY POM
    if( inPin3_State != inPin3_prev_State )
    {
      if(inPin3_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 3 zmiana: HIGH -> LOW"));
        outPin_active[1] = true;
        outPin_active[3] = true;
        prevOutputTime[1] = millis();
        prevOutputTime[3] = millis();
      }
      else  // zmiana na HIGH
      {
        Serial.println(F("InPin 3 zmiana: LOW -> HIGH"));
      }

      inPin3_prev_State = inPin3_State; // przypisz obecna wartosc
    }
    // PIN 4 KOSZ PRAWY CZER
    if( inPin4_State != inPin4_prev_State )
    {
      if(inPin4_State == LOW) // zmiana na LOW
      {
        Serial.println(F("InPin 4 zmiana: HIGH -> LOW"));
        outPin_active[1] = true;
        outPin_active[3] = true;
        prevOutputTime[1] = millis();
        prevOutputTime[3] = millis();
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
  outputCurrentTime = millis();      // pobierz czas

  // SPRAWDZA CZY FLAGA ACTIVE JEST AKTYWNA POD KAZDYM PINEM OUT
  // JESLI TAK USTAWIA NA WYJSCIU ON/LOW
  // JESLI NIE USTAWIA NA WYJSCIU OFF/HIGH
  for(int i=0; i <= outpin_array_len-1; i++)
  {
    if(outPin_active[i] == true)
    {
      digitalWriteFast(outpin[i], LOW);
      //Serial.print("for:out");
      //Serial.println(outpin[i]);

      // SPRAWDZA CZY CZAS ZOSTAL PRZEKROCZONY
      // JESLI TAK WYLACZA WYJSCIE
      // JESLI NIE - NYC
      if(outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME) // jesli minie czas OUTPUT_TIME = 8s
      {
        prevOutputTime[i] = outputCurrentTime; // zeruj licznik
        outPin_active[i] = false; // flaga output na false
      }
    }
    else // outpin == false
    {
      digitalWriteFast(outpin[i], HIGH);
    }
  }
}

void pressure_prepare()
{
  // START ODBIORU I USREDNIANIE DANYCH
  for(int i=0; i < BME_AVG_COUNT; i++) // w rozbiegu usredniaj wraz z rosnacym licznikiem i.
  {
    bme_tbl[i] = bme_data;          // przypisz dane z nadajnika x AVG COUNT
    bme_avg += bme_tbl[i];          // dodaj do sredniej wartosc z tablicy[i]
  }
  bme_avg = bme_avg / BME_AVG_COUNT;  // dzielimy przez ilosc zapisanych wartosci w tablicy
  bme_rozbieg = false;
  bme_avg_i = 0;

  /*
  if( bme_rozbieg == true )
  {
    if (bme_avg_i < BME_AVG_COUNT)  // dopoki wartosc dobije 100 probek
    {
      bme_tbl[bme_avg_i] = bme_data;  // dodaj nowa wartosc do tabeli
      bme_avg_i++;                    // zwieksz licznik
    }
    else                              // wartosc dobila do 100 = tablica pelna
    {
      bme_avg_i = 0;                  // zeruj licznik
      bme_rozbieg = false;            // rozbieg tablicy sredniej zakonczony
    }
    unsigned long start = micros();
    // Call to your function

    bme_avg = 0;
    for(int i=0; i < bme_avg_i; i++) // w rozbiegu usredniaj wraz z rosnacym licznikiem i.
    {
      bme_avg += bme_tbl[i];          // dodaj do sredniej wartosc z tablicy[i]
    }
    bme_avg = bme_avg / bme_avg_i;    // dzielimy przez ilosc zapisanych wartosci w tablicy

    // Compute the time it took
    unsigned long end = micros();
    unsigned long delta = end - start;
    Serial.println(delta);
  }
  */
}

// ODBIOR DANYCH Z NADAJNIKA I USREDNIENIE WYNIKU DO DALSZYCH DZIALAN
// URUCHAMIANE TYLKO KIEDY NADAJNIK ODBIERA BME
void manage_pressure()
{
  // ROZBIEG TABLICY SREDNIEGO CISNIENIA
  if(bme_rozbieg == true)
  {
    pressure_prepare();
    Serial.println("pressure prepare");
  }

  // START ODBIORU I USREDNIANIE DANYCH
  if(bme_avg_i < BME_AVG_COUNT)
  {
    bme_tbl[bme_avg_i] = bme_data;  // dodaj nowa wartosc do tabeli
    bme_avg_i++;                    // zwieksz licznik
  }
  else
  {
    bme_avg_i = 0;
  }

  unsigned long start = micros();
  // Call to your function

  bme_avg = 0;
  for(int i=0; i <= BME_AVG_COUNT; i++) // w rozbiegu usredniaj wraz z rosnacym licznikiem i.
  {
    bme_avg += bme_tbl[i];          // dodaj do sredniej wartosc z tablicy[i]
  }
  bme_avg = bme_avg / BME_AVG_COUNT;    // dzielimy przez ilosc zapisanych wartosci w tablicy

  // Compute the time it took
  unsigned long end = micros();
  unsigned long delta = end - start;
  Serial.println(delta);

  Serial.print("bme avg i: "); Serial.println(bme_avg_i);
  Serial.print("bme avg: "); Serial.println(bme_avg);
}

void check_pressure()
{

}

void setup() 
{
  Serial.begin(BAUDRATE);

  // PINS
  pinModeFast(INPIN1, INPUT_PULLUP);
  pinModeFast(INPIN2, INPUT_PULLUP);
  pinModeFast(INPIN3, INPUT_PULLUP);
  pinModeFast(INPIN4, INPUT_PULLUP);

  pinModeFast(OUTPIN0, OUTPUT);
  pinModeFast(OUTPIN1, OUTPUT);
  pinModeFast(OUTPIN2, OUTPUT);
  pinModeFast(OUTPIN3, OUTPUT);
  //pinModeFast(OUTPIN4, OUTPUT); // RX
  //pinModeFast(OUTPIN5, OUTPUT); // RX
  pinModeFast(OUTPIN6, OUTPUT);
  pinModeFast(OUTPIN7, OUTPUT);

  digitalWriteFast(OUTPIN0, HIGH);
  digitalWriteFast(OUTPIN1, HIGH);
  digitalWriteFast(OUTPIN2, HIGH);
  digitalWriteFast(OUTPIN3, HIGH);
  //digitalWriteFast(OUTPIN4, HIGH);  // RX
  //digitalWriteFast(OUTPIN5, HIGH);  // TX
  digitalWriteFast(OUTPIN6, HIGH);
  digitalWriteFast(OUTPIN7, HIGH);

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
    
    manage_pressure();

    // START COUNT INACTIVE TIME
    rfoffTime = millis();
  }
  else
  {
    if(millis() - rfoffTime >= RF_OFF_TIME && bme_rozbieg == false)
    {
      Serial.println("bmerozbieg true");
      bme_rozbieg = true;
    }
  }
  

  // 2. SPRAWDZ WEJSCIA IN1-IN4:
  currentTime = millis();

  if(currentTime - prevTime >= READ_REFRESH_TIME )
  {
    prevTime = currentTime;
    manage_input();
    manage_output();
  }
}
