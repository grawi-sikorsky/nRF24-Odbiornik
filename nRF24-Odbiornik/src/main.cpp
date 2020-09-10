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
#define DEBUG                   // DEBUG, SERIAL itp.
#define OUTPUT_TIME 8000        // czas wlaczenia przekaznikow po otrzymaniu sygnalu z INPUT 1-4
#define READ_REFRESH_TIME 100   // czestotliwosc ms odswiezania wejsc INPUT
#define RF_OFF_TIME 5000        // czas [ms] nieaktywnosci nadajnika po ktorym trzeba ponownie wygenerowac tablice probek cisnienia

bool inPin1_State, inPin1_prev_State,
    inPin2_State, inPin2_prev_State,
    inPin3_State, inPin3_prev_State,
    inPin4_State, inPin4_prev_State;
bool outPin_active[outpin_array_len];   // flagi informujace o stanie wyjsc
bool input_active;

// nRF24L01 DEFINICJE
const byte address[17] = "1100110011001100";  // adres nadajnika [todo: zrobic to bezpieczniejsze]
RF24 radio(9, 10); // CE, CSN 

// TIME CZASOMIERZE
time_t currentTime, prevTime = 0;                             // TIMER WEJSC INPUT - READ_REFRESH_TIME - 100 ms
time_t outputCurrentTime, prevOutputTime[outpin_array_len];   // TIMER WYJSC OUTPUT - OUTPUT_TIME - 8000 ms
time_t rfoffTime;                                             // TIMER LICZACY CZAS OD OSTATNIEGO ODBIORU DANYCH Z nRF (potrzebny m.in do pozniejszego rozbiegu sredniej)

// PRESSURE DEFINICJE I ZMIENNE
#define BME_AVG_COUNT 20      // wiecej -> dluzszy powrot avg do normy
#define BME_AVG_DIFF  800     // im mniej tym dluzej wylacza sie po dmuchaniu. Zbyt malo powoduje ze mimo wylaczenia sie gwizdka, wlacza sie ponownie gdy wartosci wracaja do normy i avg.
#define BME_AVG_SENS  200     // czulosc dmuchniecia

float bme_data;               // dane RAW prosto z nadajnika
float bme_tbl[BME_AVG_COUNT]; // tablica z probkami cisnienia 
float bme_avg = 0;            // srednie cisnienie -> bme_avg / BME_AVG_COUNT
int   bme_avg_i = 0;          // licznik AVG
bool  bme_rozbieg = true;     // info o pierwszym wypelnianiu tabeli AVG
bool  gwizd_on    = false;    // info o aktywnym gwizdku



// FUNKCJE ODBIORNIKA:


// FUNKCJA ROZBIEGOWA TABLICY Z PROBKAMI CISNIENIA
// URUCHAMIANA RAZ PO DLUZSZEJ (RF_OFF_TIME) NIEAKTYWNOSCI NADAJNIKA
void pressure_prepare()
{
  // START ODBIORU I USREDNIANIE DANYCH
  for(int i=0; i < BME_AVG_COUNT; i++)  // w rozbiegu usredniaj wraz z rosnacym licznikiem i.
  {
    bme_tbl[i] = bme_data;              // przypisz dane z nadajnika x AVG COUNT
    bme_avg += bme_tbl[i];              // dodaj do sredniej wartosc z tablicy[i]
  }
  bme_avg = bme_avg / BME_AVG_COUNT;    // dzielimy przez ilosc zapisanych wartosci w tablicy
  bme_rozbieg = false;
  bme_avg_i = 0;
}

// ODBIOR DANYCH Z NADAJNIKA I USREDNIENIE WYNIKU DO DALSZYCH DZIALAN
// URUCHAMIANE TYLKO KIEDY NADAJNIK ODBIERA BME
void manage_pressure()
{
  // ROZBIEG TABLICY SREDNIEGO CISNIENIA
  if(bme_rozbieg == true)
  {
    pressure_prepare();
    #ifdef DEBUG
      Serial.println("pressure prepare");
    #endif
  }

  // START USREDNIANIA DANYCH
  if(bme_avg_i < BME_AVG_COUNT)
  {
    if((bme_data < (bme_avg + BME_AVG_DIFF)) && (bme_data > (bme_avg - BME_AVG_DIFF)))  // jesli obecne probka miesci sie w widelkach +-[BME_AVG_DIFF] 
    {
      bme_tbl[bme_avg_i] = bme_data;  // dodaj nowa wartosc do tabeli
      bme_avg_i++;                    // zwieksz licznik
    }
  }
  else
  {
    bme_avg_i = 0;
  }

  bme_avg = 0;                          // zeruj srednia przed petla
  for(int i=0; i <= BME_AVG_COUNT; i++) // Usredniaj zgodnie z iloscia probek [BME_AVG_COUNT]
  {
    bme_avg += bme_tbl[i];              // dodaj do sredniej wartosc z tablicy[i]
  }
  bme_avg = bme_avg / BME_AVG_COUNT;    // dzielimy przez ilosc zapisanych wartosci w tablicy
}

// SPRAWDZA CZY NASTAPIL WZROST CISNIENIA W STOSUNKU DO SREDNIEJ AVG
// CZULOSC BME_AVG_SENS
void check_pressure()
{
  if(bme_data > (bme_avg + BME_AVG_SENS))             // JESLI NOWA PROBKA JEST WIEKSZA OD SREDNIEJ [AVG + AVG_DIFF]
  {
    #ifdef DEBUG
      Serial.print("GWIZD ON: "); Serial.println(bme_data);
    #endif

    gwizd_on = true;                                  // ustaw gwizdek aktywny

    if(input_active == true)                          // jesli OUTPUT byl zaswiecony przez wejscia INPUT
    {
      for(int i=0; i <= outpin_array_len-1; i++)        // na chwile wyzeruj wszystkie wyjscia -> migniecie gdyby wyjscia byly juz aktywne sygnalem z WEJSC.
      {
        outPin_active[i] = false;                       // wszystkie flagi off
        digitalWriteFast(outpin[i], HIGH);              // wszystkie piny off
      }
      input_active = false;                            // Zapomnij o INPUTach
    }

    outPin_active[0] = true;                          // ustaw konkretne wyjscia aktywne po gwizdnieciu [do konfiguracji]
    outPin_active[1] = true;
    outPin_active[2] = true;
    outPin_active[3] = true;
  }
  else if((bme_data < (bme_avg + BME_AVG_DIFF)) && (bme_data > (bme_avg - BME_AVG_DIFF)) && gwizd_on == true)   // JESLI CISNIENIE WRACA DO WIDELEK [AVG +- AVG_DIFF] a gwizdek jest aktywny
  {
    #ifdef DEBUG
      Serial.println("Gwizd OFF");
    #endif
    outPin_active[0] = false;                         // aktywowane gwizdkiem wyjscia ustaw na OFF
    outPin_active[1] = false;
    outPin_active[2] = false;
    outPin_active[3] = false;
    gwizd_on = false;                                 // flaga gwizdka rowniez OFF
  }
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

// Obsluga wejsc INPUT i przypisanie odpowiednim WYJSCIOM flagi aktywnej
// Obsluga flag w manage_output()
// konfiguracja:
// KOSZ LEWY POM && KOSZ LEWY CZER = KOSZ LEWY LED && PODLOGA LEWY LED
// KOSZ PRAWY POM && KOSZ PRAWY CZER = KOSZ PRAWY LED && PODLOGA PRAWY LED
void manage_input()
{
  if (read_input_pins() == true)                              // jesli byla zmiana na wejsciach sprawdza kazde po kolei i przypisuje konkretne wyjscia 
  {
    // PIN 1 KOSZ LEWY POM
    if( inPin1_State != inPin1_prev_State )                   // jesli nastapila zmiana:
    {
      if(inPin1_State == LOW)                                 // i byla to zmiana na LOW czyli aktywne
      {
        #ifdef DEBUG
          Serial.println(F("InPin 1 zmiana: HIGH -> LOW"));
        #endif
        outPin_active[0] = true;
        outPin_active[2] = true;
        prevOutputTime[0] = millis();
        prevOutputTime[2] = millis();
        input_active = true;
      }
      else                                                    // zmiana na HIGH
      {
        #ifdef DEBUG
          Serial.println(F("InPin 1 zmiana: LOW -> HIGH"));
        #endif
      }
      inPin1_prev_State = inPin1_State;                       // przypisz obecna wartosc
    }
    // PIN 2 KOSZ LEWY CZER
    if( inPin2_State != inPin2_prev_State )
    {
      if(inPin2_State == LOW) // zmiana na LOW
      {
        #ifdef DEBUG
          Serial.println(F("InPin 2 zmiana: HIGH -> LOW"));
        #endif
        outPin_active[0] = true;
        outPin_active[2] = true;
        prevOutputTime[0] = millis();
        prevOutputTime[2] = millis();
        input_active = true;
      }
      else  // zmiana na HIGH
      {
        #ifdef DEBUG
          Serial.println(F("InPin 2 zmiana: LOW -> HIGH"));
        #endif
      }
      inPin2_prev_State = inPin2_State; // przypisz obecna wartosc
    }
    // PIN 3 KOSZ PRAWY POM
    if( inPin3_State != inPin3_prev_State )
    {
      if(inPin3_State == LOW) // zmiana na LOW
      {
        #ifdef DEBUG
          Serial.println(F("InPin 3 zmiana: HIGH -> LOW"));
        #endif
        outPin_active[1] = true;
        outPin_active[3] = true;
        prevOutputTime[1] = millis();
        prevOutputTime[3] = millis();
        input_active = true;
      }
      else  // zmiana na HIGH
      {
        #ifdef DEBUG
          Serial.println(F("InPin 3 zmiana: LOW -> HIGH"));
        #endif
      }
      inPin3_prev_State = inPin3_State; // przypisz obecna wartosc
    }
    // PIN 4 KOSZ PRAWY CZER
    if( inPin4_State != inPin4_prev_State )
    {
      if(inPin4_State == LOW) // zmiana na LOW
      {
        #ifdef DEBUG
          Serial.println(F("InPin 4 zmiana: HIGH -> LOW"));
        #endif
        outPin_active[1] = true;
        outPin_active[3] = true;
        prevOutputTime[1] = millis();
        prevOutputTime[3] = millis();
        input_active = true;
      }
      else  // zmiana na HIGH
      {
        #ifdef DEBUG
          Serial.println(F("InPin 4 zmiana: LOW -> HIGH"));
        #endif
      }
      inPin4_prev_State = inPin4_State; // przypisz obecna wartosc
    }
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
      if((outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME) && gwizd_on == false) // jesli minie czas OUTPUT_TIME A GWIZDEK NIE JEST AKTYWNY!
      {
        prevOutputTime[i] = outputCurrentTime; // zeruj licznik
        outPin_active[i] = false; // flaga output na false
        input_active = false;     // info o wejsciu
      }
    }
    else if(outPin_active[i] == false && gwizd_on == false) // outpin == false
    {
      digitalWriteFast(outpin[i], HIGH);
    }
  }
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
  if (radio.available())                                        // jesli dane sa dostepne ->
  {
    radio.read(&bme_data, sizeof(bme_data));                    // pobierz cisnienie z nadajnika
    #ifdef DEBUG
      Serial.print("Odebrano: "); Serial.println(bme_data);
      Serial.print("AVG: "); Serial.println(bme_avg);
    #endif
    
    manage_pressure();                                          // zarzadzanie cisnieniem i srednia
    check_pressure();                                           // pomiar czy nastapil wzrost

    // START COUNT INACTIVE TIME
    rfoffTime = millis();                                       // odliczaj czas od tego momentu gdy nie pojawi sie kolejna porcja danych
  }
  else                                                          // jesli danych z nRF24 brak ->
  {
    if(millis() - rfoffTime >= RF_OFF_TIME && bme_rozbieg == false)   // sprawdz czas od ostatniego pakietu
    {
      #ifdef DEBUG
        Serial.println("BME rozbieg true");
      #endif
      bme_rozbieg = true;                                             // ustaw informacje o potrzebie ponownego wypelnienia tablicy danymi
    }
  }

  // 2. SPRAWDZ WEJSCIA IN1-IN4:
  currentTime = millis();                                       // pobierz czas do odliczania interwalow sprawdzania WEJSC

  if(currentTime - prevTime >= READ_REFRESH_TIME )              // jesli minelo [READ_REFRESH_TIME] ->
  {
    prevTime = currentTime;
    manage_input();                                             // zarzadzaj wejsciami
    manage_output();                                            // zarzadzaj wyjsciami
  }
}
