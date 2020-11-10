#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <time.h>

#define BAUDRATE 250000

// KONFIGURACJA
#define DEBUG                   // DEBUG, SERIAL itp.
#define DEBUGSERIAL             // DEBUG SERIAL - usuwa 2 output piny dla RX/TX
#define OUTPUT_TIME 4000        // czas wlaczenia przekaznikow po otrzymaniu sygnalu z INPUT 1-4
#define OUTPUT_GWIZD_TIME 2000  // czas wlaczenia przekaznikow po otrzymaniu GWIZDNIECIA
#define READ_REFRESH_TIME 100   // czestotliwosc ms odswiezania wejsc INPUT
#define RF_OFF_TIME 5000        // czas [ms] nieaktywnosci nadajnika po ktorym trzeba ponownie wygenerowac tablice probek cisnienia
#define RF_SENDBACK 25
#define TIMEOUT_1       20000       // pierwszy timeiut // realnie wychodzi jakies (1 800 000 ms = 30 min) / 25 = 72000
#define TIMEOUT_2       40000       // drugi prog = 5 400 000 = 90 min // z uwagi na sleep-millis: 60 min

// KONFIGURACJA WYJSC:
//#define TEST_MODE                 // TESTMODE TO WERSJA PODSTAWOWA: EWRYFINK FAKING ILUMINEJTED
#define EASY_MODE                 // WESJA PODSTAWOWA LANCUT [ GWIZDEK -> OUT1; POMOCNICZE -> OUT2; Reszta wolna]
#define DEFAULT_MODE              // DOMYSLNE USTAWIENIA
#define GWIZD_2S                  // GWIZD = 2s LEDOW - wylacza sie automatycznie, jesli nie zdefiniowane to nadajnik wysyla 1, potem 0 aby wylaczyc ledy, a nastepnie 2 jako brak transmisji.

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

#ifdef DEBUGSERIAL
  int outpin[] = {OUTPIN0, OUTPIN1, OUTPIN2, OUTPIN3, OUTPIN6, OUTPIN7}; // UP OUTPIN4, OUTPIN5
#else
  int outpin[] = {OUTPIN0, OUTPIN1, OUTPIN2, OUTPIN3, OUTPIN4, OUTPIN5, OUTPIN6, OUTPIN7}; // UP OUTPIN4, OUTPIN5
#endif

const int outpin_array_len = (sizeof(outpin)/sizeof(*outpin));  // DLUGOSC TABLICY PINOW OUTPUT

#define NEOPIN  4   // NEOPIXEL
#define LEDPIN  14  // LED
#define BTNPIN  15  // w przyszlosci PC1/ pin 15, obecnie A7/ADC7 pin A7

bool inPin1_State, inPin1_prev_State,
    inPin2_State, inPin2_prev_State,
    inPin3_State, inPin3_prev_State,
    inPin4_State, inPin4_prev_State;
bool outPin_active[outpin_array_len];   // flagi informujace o stanie wyjsc
bool outPin_input[outpin_array_len];    // flagi informujace o zrodle zalaczenia wyjscia - jesli true to jest sa to wejscia INPUT, jesli false to Gwizdek
bool input_rf;                          // info z nadajnika rf o ledach

// nRF24L01 DEFINICJE
const byte address[5] = "Odb1";  // domyslny adres odbiornika
RF24 radio(9, 10); // CE, CSN 

// TIME CZASOMIERZE
time_t currentTime, prevTime = 0;                             // TIMER WEJSC INPUT - READ_REFRESH_TIME - 100 ms
time_t outputCurrentTime, prevOutputTime[outpin_array_len];   // TIMER WYJSC OUTPUT - OUTPUT_TIME - 5000 ms
time_t rfoffTime;                                             // TIMER LICZACY CZAS OD OSTATNIEGO ODBIORU DANYCH Z nRF (potrzebny m.in do pozniejszego rozbiegu sredniej)
time_t timeout_start_at;                                      // TIMER LICZACY CZAS OD OSTATNIEGO GWIZDNIECIA.

// PRESSURE DEFINICJE I ZMIENNE
#define BME_AVG_COUNT 20      // wiecej -> dluzszy powrot avg do normy
#define BME_AVG_DIFF  800     // im mniej tym dluzej wylacza sie po dmuchaniu. Zbyt malo powoduje ze mimo wylaczenia sie gwizdka, wlacza sie ponownie gdy wartosci wracaja do normy i avg.
#define BME_AVG_SENS  200     // czulosc dmuchniecia

struct outdata
{
  int     getgwizd;
  float   raw;
  float   avg;
};
outdata nrfdata;

bool ackOK = true;

float bme_tbl[BME_AVG_COUNT]; // tablica z probkami cisnienia 
float bme_avg = 0;            // srednie cisnienie -> bme_avg / BME_AVG_COUNT
int   bme_avg_i = 0;          // licznik AVG
bool  bme_rozbieg = true;     // info o pierwszym wypelnianiu tabeli AVG
bool  gwizd_on    = false;    // info o aktywnym gwizdku
bool  slowtime;               // info zwrotne o zwolnieniu nadajnika
bool  sleeptime;              // info zwrotne o uspieniu nadajnika

// FUNKCJE ODBIORNIKA:

// nowa sprawa
#define INFORMUJ_INNE_ODBIORNIKI

#ifdef INFORMUJ_INNE_ODBIORNIKI
  void send_RF_to_other()
  {
    // przerzucamy sie na nadawanie
    radio.stopListening();
    radio.openWritingPipe(address);
    radio.enableAckPayload();
    radio.setRetries(1,8); // delay, count
    radio.setChannel(95);

    // wysylamy wiadomosc
    bool result;
    result = radio.write(&nrfdata, sizeof(nrfdata));   // PIERWSZA TRANSMISJA DO ODBIORNIKA!

    // przerzucamy sie na odbior
    radio.openReadingPipe(1, address);
    radio.setChannel(95);
    radio.startListening();
  }
#endif
// SPRAWDZA CZY Z NADAJNIKA DOTARLA WARTOSC TRUE DLA GETGWIZD
// JESLI TAK WLACZA PRZEKAZNIKI ITP..
void check_whistle()
{
  if(nrfdata.getgwizd == 1)                 // JESLI NOWA PROBKA JEST WIEKSZA OD SREDNIEJ [AVG + AVG_DIFF]
  {
    #ifdef INFORMUJ_INNE_ODBIORNIKI
      // FUNKCJA ODSYLAJACA DO POZOSTALYCH ODBIORNIKOW INFO ZE MAJA SIE WZBUDZIC
      send_RF_to_other();
    #endif

    timeout_start_at = millis();                      // ustaw czas ostatniego gwizdniecia

    // TEST_MODE
    #if defined TEST_MODE
      outPin_active[0] = true;                          // ustaw konkretne wyjscia aktywne po gwizdnieciu [do konfiguracji]
      outPin_active[1] = true;
      outPin_active[2] = true;
      outPin_active[3] = true;
      outPin_active[4] = true;
      outPin_active[5] = true;
      #ifndef DEBUGSERIAL
      outPin_active[6] = true;
      outPin_active[7] = true;
      #endif

    // EASY_MODE
    #elif defined EASY_MODE
      outPin_active[0] = true;

    // NORMAL_MODE
    #else
      outPin_active[0] = true;
      // costam costam
    #endif
  }
  else if(nrfdata.getgwizd == 0)   // JESLI CISNIENIE WRACA DO WIDELEK [AVG +- AVG_DIFF] a gwizdek jest aktywny
  {
    // TEST_MODE
    #if defined TEST_MODE
      outPin_active[0] = false;                          // ustaw konkretne wyjscia aktywne po gwizdnieciu [do konfiguracji]
      outPin_active[1] = false;
      outPin_active[2] = false;
      outPin_active[3] = false;
      outPin_active[4] = false;
      outPin_active[5] = false;
      #ifndef DEBUGSERIAL
      outPin_active[6] = false;
      outPin_active[7] = false;
      #endif

    // EASY_MODE
    #elif defined EASY_MODE
      outPin_active[0] = false;

    // NORMAL_MODE
    #else
      outPin_active[0] = false;
      // costam costam
    #endif
  }
  else if(nrfdata.getgwizd == 2)  // transmisja off
  {
    // nyc?
  }
}

// USTAWIA OUTPUTY W JEDNEJ FUNKCJI
// PRZYJMUJE NUMER WYJSCIA KTORY CHCEMY AKTYWOWAC [ 0 - 7 ]
// WIEKSZA WARTOSC ZOSTAJE POMINIETA
// DEBUG SERIAL POWODUJE PRZEPELNIENIE TABLICY !!!
#ifdef DEBUGSERIAL
void set_output(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10)
#else
void set_output(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10, int g=10, int h=10)
#endif
{
    if(a<8) { outPin_active[a] = true; prevOutputTime[a] = millis(); outPin_input[a] = true; }
    if(b<8) { outPin_active[b] = true; prevOutputTime[b] = millis(); outPin_input[b] = true; }
    if(c<8) { outPin_active[c] = true; prevOutputTime[c] = millis(); outPin_input[c] = true; }
    if(d<8) { outPin_active[d] = true; prevOutputTime[d] = millis(); outPin_input[d] = true; }
    if(e<8) { outPin_active[e] = true; prevOutputTime[e] = millis(); outPin_input[e] = true; }
    if(f<8) { outPin_active[f] = true; prevOutputTime[f] = millis(); outPin_input[f] = true; }
    #ifndef DEBUGSERIAL
    if(g<8) { outPin_active[g] = true; prevOutputTime[g] = millis(); outPin_input[g] = true; }
    if(h<8) { outPin_active[h] = true; prevOutputTime[h] = millis(); outPin_input[h] = true; }
    #endif
}

// Sprawdza czy stan wejsc sie zmienil w stosunku do prev_state
// domyslnie OFF -> HIGH  // ON -> LOW
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
    return true;                              // jesli na ktorymkolwiek pinie wystapia zmiana zwroc TRUE
  else 
    return false;                             // jesli bez zmian -> FALSE
}

// Sprawdza czy w danych z RF pojawily sie wartosci 11 12 13 21 22 23.
bool read_input_rf()
{
  if( nrfdata.getgwizd == 11 || nrfdata.getgwizd == 12 || nrfdata.getgwizd == 13 ||
      nrfdata.getgwizd == 21 || nrfdata.getgwizd == 22 || nrfdata.getgwizd == 23 )
    {
      return true;
    }
  return false;
}

// Obsluga wejsc INPUT i przypisanie odpowiednim WYJSCIOM flagi aktywnej
// Obsluga wejsc z nadajnika pomocniczego RF
// Tutaj jedynie ustawiamy co ma zostac wlaczone -> czesc wykonawcza w manage_output()
// konfiguracja:
// KOSZ LEWY POM && KOSZ LEWY CZER = KOSZ LEWY LED && PODLOGA LEWY LED
// KOSZ PRAWY POM && KOSZ PRAWY CZER = KOSZ PRAWY LED && PODLOGA PRAWY LED
void manage_input()
{
  // CZESC ODPOWIEDZIALNA ZA INPUT Z WEJSC FIZYCZNYCH ODBIORNIKA!
  if (read_input_pins() == true )  // jesli byla zmiana na wejsciach sprawdza kazde po kolei i przypisuje konkretne wyjscia 
  {
    // PIN 1 KOSZ LEWY POM
    if( inPin1_State != inPin1_prev_State )                   // jesli nastapila zmiana:
    {
      if(inPin1_State == LOW)                                 // i byla to zmiana na LOW czyli aktywne
      {
        set_output( 0, 1 );
      }
      inPin1_prev_State = inPin1_State;                       // przypisz obecna wartosc
    }
    // PIN 2 KOSZ LEWY CZER
    if( inPin2_State != inPin2_prev_State )
    {
      if(inPin2_State == LOW) // zmiana na LOW
      {
        set_output( 2, 3 );
      }
      inPin2_prev_State = inPin2_State; // przypisz obecna wartosc
    }
    // PIN 3 KOSZ PRAWY POM
    if( inPin3_State != inPin3_prev_State )
    {
      if(inPin3_State == LOW) // zmiana na LOW
      {
        set_output( 4, 5 );
      }
      inPin3_prev_State = inPin3_State; // przypisz obecna wartosc
    }
    // PIN 4 KOSZ PRAWY CZER
    if( inPin4_State != inPin4_prev_State )
    {
      if(inPin4_State == LOW) // zmiana na LOW
      {
        set_output( 6, 7 );
      }
      inPin4_prev_State = inPin4_State; // przypisz obecna wartosc
    }
  }

  // CZEŚĆ ODPOWIEDZIALNA ZA INPUT Z NADAJNIKOW POMOCNICZYCH!
  if (read_input_rf() == true )
  {
    #if defined (TEST_MODE)
      #ifdef DEBUGSERIAL
        set_output(0,1,2,3,4,5);  // JESLI TESTMODE (EWRYFINK IS ILUMINEJTED!)
      #else
        set_output(0,1,2,3,4,5,6,7);  // JESLI TESTMODE (EWRYFINK IS ILUMINEJTED!)
      #endif
    #elif defined (EASY_MODE)
      set_output(1);

    #else                           // JESLI KONFIGURACJA DOMYSLNA
      if( nrfdata.getgwizd == 11)
      {
        set_output(0,1);
      }
      else if ( nrfdata.getgwizd == 12 )
      {
        set_output(2,3);
      }
      else if ( nrfdata.getgwizd == 13 )
      {
        set_output(0,1,2,3);
      }

      if( nrfdata.getgwizd == 21 )
      {
        set_output(4,5);
      }
      else if ( nrfdata.getgwizd == 22 )
      {
        set_output(6,7);
      }
      else if ( nrfdata.getgwizd == 23 )
      {
        set_output(4,5,6,7);
      }
    #endif

    nrfdata.getgwizd = 2; // default state...
  }
}

// OBSLUGA WYJSC BAZUJE NA INFORMACJACH ZDOBYTYCH W MANAGE_INPUT()
// USTAWIA STANY WYSOKIE I NISKIE W ZALEZNOSCI OD AKTYWNYCH FLAG Z TABLICY outPin_active[]
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
      // SPRAWDZA CZY CZAS ZOSTAL PRZEKROCZONY
      // JESLI TAK WYLACZA WYJSCIE
      // JESLI NIE - NYC
      if((outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME) && outPin_input[i] == true) // jesli minie czas OUTPUT_TIME A GWIZDEK NIE JEST AKTYWNY!
      {
        prevOutputTime[i] = outputCurrentTime; // zeruj licznik
        outPin_active[i] = false; // flaga output na false
        outPin_input[i] = false; // info o wejsciu
      }
      #ifdef GWIZD_2S
      if((outputCurrentTime - timeout_start_at >= OUTPUT_GWIZD_TIME) && outPin_input[i] == false)
      {
        timeout_start_at = outputCurrentTime; // zeruj licznik
        outPin_active[i] = false; // flaga output na false
        outPin_input[i] = false; // info o wejsciu
      }
      #endif
    }
    else if(outPin_active[i] == false && gwizd_on == false) // outpin == false
    {
      digitalWriteFast(outpin[i], HIGH);
    }
  }
}

// SETUP
void setup() 
{
  #ifdef DEBUGSERIAL
    Serial.begin(BAUDRATE);
  #endif

  // PINS
  pinModeFast(INPIN1, INPUT_PULLUP);
  pinModeFast(INPIN2, INPUT_PULLUP);
  pinModeFast(INPIN3, INPUT_PULLUP);
  pinModeFast(INPIN4, INPUT_PULLUP);

  pinModeFast(OUTPIN0, OUTPUT);
  pinModeFast(OUTPIN1, OUTPUT);
  pinModeFast(OUTPIN2, OUTPUT);
  pinModeFast(OUTPIN3, OUTPUT);
  #ifndef DEBUGSERIAL
  pinModeFast(OUTPIN4, OUTPUT); // RX
  pinModeFast(OUTPIN5, OUTPUT); // RX
  #endif
  pinModeFast(OUTPIN6, OUTPUT);
  pinModeFast(OUTPIN7, OUTPUT);

  digitalWriteFast(OUTPIN0, HIGH);
  digitalWriteFast(OUTPIN1, HIGH);
  digitalWriteFast(OUTPIN2, HIGH);
  digitalWriteFast(OUTPIN3, HIGH);
  #ifndef DEBUGSERIAL
  digitalWriteFast(OUTPIN4, HIGH);  // RX
  digitalWriteFast(OUTPIN5, HIGH);  // TX
  #endif
  digitalWriteFast(OUTPIN6, HIGH);
  digitalWriteFast(OUTPIN7, HIGH);

  inPin1_State = inPin1_prev_State = digitalReadFast(INPIN1);   // zakladamy ze stan bedzie spoczynkowy (!)
  inPin2_State = inPin2_prev_State = digitalReadFast(INPIN2); 
  inPin3_State = inPin3_prev_State = digitalReadFast(INPIN3); 
  inPin4_State = inPin4_prev_State = digitalReadFast(INPIN4); 

  // nRF24L01
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.enableAckPayload();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(95);
  //radio.setRetries(5,15); // delay, count
  //radio.setPayloadSize();

  radio.startListening();
  pinMode(10, OUTPUT);  // ?????

  #ifdef DEBUGSERIAL
    Serial.println("SETUP:OK");
  #endif
}

// LOOP
void loop() {

  // 1. SPRAWDZ TRANSMISJE RADIOWA OD GWIZDKA:
  if (radio.available())                                        // jesli dane sa dostepne ->
  {
    digitalWriteFast(LEDPIN,HIGH);                            // INFO LED ON
    radio.read(&nrfdata, sizeof(nrfdata));                    // pobierz cisnienie z nadajnika

    #ifdef DEBUGSERIAL
      Serial.print("GwizdON: "); Serial.println(nrfdata.getgwizd);
    #endif

    check_whistle();                                              // pomiar czy nastapil wzrost

    // START COUNT INACTIVE TIME
    rfoffTime = millis();                                       // odliczaj czas od tego momentu gdy nie pojawi sie kolejna porcja danych
  }
  else                                                          // jesli danych z nRF24 brak ->
  {
    if(millis() - rfoffTime >= RF_OFF_TIME && bme_rozbieg == false)   // sprawdz czas od ostatniego pakietu
    {
      #ifdef DEBUGSERIAL
        Serial.println("ostatni pakiet RF_OFF_TIME temu");
      #endif
    }

    //TIMEOUT DLA INFO LEDA
    if(millis() - rfoffTime >= 50)
    {
      digitalWriteFast(LEDPIN,LOW);
    }
  }

  // 2. SPRAWDZ WEJSCIA IN1-IN4:
  currentTime = millis();                                       // pobierz czas do odliczania interwalow sprawdzania WEJSC

  if(currentTime - prevTime >= READ_REFRESH_TIME )              // jesli minelo [READ_REFRESH_TIME] ->
  {
    prevTime = currentTime;
    manage_input();                                             // zarzadzaj wejsciami
    manage_output();                                            // zarzadzaj wyjsciami

    Serial.println("loop");
  }
}
