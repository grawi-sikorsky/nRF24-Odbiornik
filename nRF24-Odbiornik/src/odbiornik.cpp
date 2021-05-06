#include "../src/odbiornik.h"

RF24 radio(9, 10); // CE, CSN 


bool Odbiornik::getActive_output_pin(int i)
{
  return Odbiornik::output_active[i];
}
void Odbiornik::setActive_output_pin(int i, bool val)
{
  Odbiornik::output_active[i] = val;
}

void Odbiornik::init()
{
  // PINS
  pinModeFast(INPIN1, INPUT_PULLUP);
  pinModeFast(ADDR1, INPUT_PULLUP);
  pinModeFast(ADDR2, INPUT_PULLUP);
  pinModeFast(ADDR3, INPUT_PULLUP);

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
}

void Odbiornik::initRF()
{
  // nRF24L01
  radio.begin();
  radio.openReadingPipe(1, address[address_nr]);
  //radio.enableAckPayload();
  //radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(95);
  //radio.setRetries(5,15); // delay, count
  //radio.setPayloadSize();

  radio.startListening();
  pinMode(10, OUTPUT);  // ?????
}

void Odbiornik::setInfoLED(bool state)
{
  digitalWriteFast(LEDPIN, state);
}

void Odbiornik::check_whistle()
{
    if(nrfdata.getgwizd == 1)                 // JESLI GWIZDEK WYSLAL SYGNAL O WZROSCIE CISNIENIA
  {
    #ifdef INFORMUJ_INNE_ODBIORNIKI
      // FUNKCJA ODSYLAJACA DO POZOSTALYCH ODBIORNIKOW INFO ZE MAJA SIE WZBUDZIC - discontinued
    #endif

    timeout_start_at = millis();                        // ustaw czas ostatniego gwizdniecia

    // TEST_MODE
    #if defined TEST_MODE
      output_active[0] = true;                          // ustaw konkretne wyjscia aktywne po gwizdnieciu [do konfiguracji]
      output_active[1] = true;
      output_active[2] = true;
      output_active[3] = true;
      output_active[4] = true;
      output_active[5] = true;
      #ifndef DEBUGSERIAL
      output_active[6] = true;
      output_active[7] = true;
      #endif

    // EASY_MODE
    #elif defined EASY_MODE
      output_active[0] = true;                          // Obecnie klient potrzebuje wylacznie jednego przekaznika
      //set_output_strobo(1);
    // NORMAL_MODE
    #else
      output_active[0] = true;
      // costam costam
    #endif
  }
  else if(nrfdata.getgwizd == 0)    // JESLI GWIZDEK WYSLAL SYGNAL O SPADKU CISNIENIA - WYLACZAMY PRZEKAZNIKI
  {
    // TEST_MODE
    #if defined TEST_MODE
      output_active[0] = false;                          // ustaw konkretne wyjscia aktywne po gwizdnieciu [do konfiguracji]
      output_active[1] = false;
      output_active[2] = false;
      output_active[3] = false;
      output_active[4] = false;
      output_active[5] = false;
      #ifndef DEBUGSERIAL
      output_active[6] = false;
      output_active[7] = false;
      #endif

    // EASY_MODE
    #elif defined EASY_MODE
      output_active[0] = false;

    // NORMAL_MODE
    #else
      output_active[0] = false;
      // costam costam
    #endif
  }
  else if(nrfdata.getgwizd == 2)  // transmisja off
  {
    // nyc ne robymy..
  }
  else // jesli pojawi sie jakies garbage...
  {
    // nrfdata.getgwizd = 2;
    // bylo nrfdata = 2 ale podczas otrzymywania z pomocniczych 11/12/13/21/22/23 ustawialo 2 - trzeba odliftrowac
    // TODO: odfiltrowac wszystkie inne sygnaly niz uzywane w RF
  }
}


// USTAWIA OUTPUTY W JEDNEJ FUNKCJI
// PRZYJMUJE NUMER WYJSCIA KTORY CHCEMY AKTYWOWAC [ 0 - 7 ]
// WIEKSZA WARTOSC ZOSTAJE POMINIETA
#ifdef DEBUGSERIAL
void Odbiornik::set_output(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10)
#else
void Odbiornik::set_output(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10, int g=10, int h=10)
#endif
{
    if(a<8) { output_active[a] = true; prevOutputTime[a] = millis(); input_source[a] = true; }
    if(b<8) { output_active[b] = true; prevOutputTime[b] = millis(); input_source[b] = true; }
    if(c<8) { output_active[c] = true; prevOutputTime[c] = millis(); input_source[c] = true; }
    if(d<8) { output_active[d] = true; prevOutputTime[d] = millis(); input_source[d] = true; }
    if(e<8) { output_active[e] = true; prevOutputTime[e] = millis(); input_source[e] = true; }
    if(f<8) { output_active[f] = true; prevOutputTime[f] = millis(); input_source[f] = true; }
    #ifndef DEBUGSERIAL
    if(g<8) { output_active[g] = true; prevOutputTime[g] = millis(); input_source[g] = true; }
    if(h<8) { output_active[h] = true; prevOutputTime[h] = millis(); input_source[h] = true; }
    #endif
}

#ifdef DEBUGSERIAL
void Odbiornik::set_output_strobo(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10)
#else
void Odbiornik::set_output_strobo(int a=10, int b=10, int c=10, int d=10, int e=10, int f=10, int g=10, int h=10)
#endif
{
    if(a<8) { output_active[a] = true; prevOutputTime[a] = millis(); input_source[a] = true; output_strobo[a]=true; }
    if(b<8) { output_active[b] = true; prevOutputTime[b] = millis(); input_source[b] = true; output_strobo[b]=true; }
    if(c<8) { output_active[c] = true; prevOutputTime[c] = millis(); input_source[c] = true; output_strobo[c]=true; }
    if(d<8) { output_active[d] = true; prevOutputTime[d] = millis(); input_source[d] = true; output_strobo[d]=true; }
    if(e<8) { output_active[e] = true; prevOutputTime[e] = millis(); input_source[e] = true; output_strobo[e]=true; }
    if(f<8) { output_active[f] = true; prevOutputTime[f] = millis(); input_source[f] = true; output_strobo[f]=true; }
    #ifndef DEBUGSERIAL
    if(g<8) { output_active[g] = true; prevOutputTime[g] = millis(); input_source[g] = true; output_strobo[g]=true; }
    if(h<8) { output_active[h] = true; prevOutputTime[h] = millis(); input_source[h] = true; output_strobo[h]=true; }
    #endif
}


// Sprawdza czy w danych z RF pojawily sie wartosci 11 12 13 21 22 23.
// Sa to sygnaly z nadajnika pomocniczego [11,12,13 - LEWY] [21,22,23 - PRAWY]
bool Odbiornik::read_input_rf()
{
  if( nrfdata.getgwizd == 11 || nrfdata.getgwizd == 12 || nrfdata.getgwizd == 13 ||
      nrfdata.getgwizd == 21 || nrfdata.getgwizd == 22 || nrfdata.getgwizd == 23 )
    {
      return true;
    }
  return false;
}

bool Odbiornik::read_input_pins()
{
  inPin1_State = digitalReadFast(INPIN1);
  //inPin2_State = digitalReadFast(INPIN2); // wylaczone bo 3 piny sa przeznaczone na adresy, pozostalo tylko jedno wejscie fizyczne w odbiorniku
  //inPin3_State = digitalReadFast(INPIN3);
  //inPin4_State = digitalReadFast(INPIN4);

  if( inPin1_State != inPin1_prev_State )
  //||  inPin2_State != inPin2_prev_State || 
  //    inPin3_State != inPin3_prev_State || 
  //    inPin4_State != inPin4_prev_State )
    return true;                              // jesli na ktorymkolwiek pinie wystapia zmiana zwroc TRUE
  else 
    return false;                             // jesli bez zmian -> FALSE
}

// Obsluga wejsc INPUT i przypisanie odpowiednim WYJSCIOM flagi aktywnej
// Obsluga wejsc z nadajnika pomocniczego RF
// Tutaj jedynie ustawiamy co ma zostac wlaczone -> czesc wykonawcza w manage_output()
// konfiguracja:
// KOSZ LEWY POM && KOSZ LEWY CZER = KOSZ LEWY LED && PODLOGA LEWY LED
// KOSZ PRAWY POM && KOSZ PRAWY CZER = KOSZ PRAWY LED && PODLOGA PRAWY LED
void Odbiornik::manage_input()
{
  // CZESC ODPOWIEDZIALNA ZA INPUT Z WEJSC FIZYCZNYCH ODBIORNIKA!
  if (read_input_pins() == true )  // jesli byla zmiana na wejsciach sprawdza kazde po kolei i przypisuje konkretne wyjscia 
  {
    // INPUT PIN 1 - 
    if( inPin1_State != inPin1_prev_State )                   // jesli nastapila zmiana:
    {
      if(inPin1_State == LOW)                                 // i byla to zmiana na LOW czyli aktywne
      {
        set_output( 0, 1 );
      }
      inPin1_prev_State = inPin1_State;                       // przypisz obecna wartosc
    }
    /*
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
    */
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
      set_output_strobo(1);

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
// USTAWIA STANY WYSOKIE I NISKIE W ZALEZNOSCI OD AKTYWNYCH FLAG Z TABLICY output_active[]
void Odbiornik::manage_output()
{
  outputCurrentTime = millis();      // pobierz czas

  // SPRAWDZA CZY FLAGA ACTIVE JEST AKTYWNA POD KAZDYM PINEM OUT
  // JESLI TAK USTAWIA NA WYJSCIU ON/LOW
  // JESLI NIE USTAWIA NA WYJSCIU OFF/HIGH
  for(int i=0; i <= outpin_array_len-1; i++)
  {
    if(output_active[i] == true && output_strobo[i] == false)
    {
      digitalWriteFast(outpin[i], LOW); // uruchamiamy przekaznik ON
      // SPRAWDZA CZY CZAS ZOSTAL PRZEKROCZONY
      // JESLI TAK WYLACZA WYJSCIE
      // JESLI NIE - NYC
      if((outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME) && input_source[i] == true) // jesli minie czas OUTPUT_TIME A GWIZDEK NIE JEST AKTYWNY!
      {
        prevOutputTime[i] = outputCurrentTime; // zeruj licznik
        output_active[i] = false; // flaga output na false
        input_source[i] = false; // info o wejsciu
      }
      #ifdef GWIZD_2S
      if((outputCurrentTime - timeout_start_at >= OUTPUT_GWIZD_TIME) && input_source[i] == false)
      {
        timeout_start_at = outputCurrentTime; // zeruj licznik
        output_active[i] = false; // flaga output na false
        input_source[i] = false; // info o wejsciu
      }
      #endif
    }
    else if(output_strobo[i] == true) // flaga strobo powinna wystarczyc? TODO
    {
      //digitalWriteFast(outpin[i], LOW); // uruchamiamy przekaznik ON

      if((outputCurrentTime - outputStroboTime[i] >= STROBO_FREQ) && input_source[i] == true)
      {
        digitalWriteFast(LEDPIN, !digitalReadFast(LEDPIN)); // odwroc stan 
        output_active[i] = false;
        digitalWriteFast(outpin[i], !digitalReadFast(outpin[i])); // uruchamiamy przekaznik ON
        outputStroboTime[i] = outputCurrentTime;
      }

      if((outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME) && input_source[i] == true)
      {
        output_strobo[i] = false; // wylaczamy strobo
      }
    }
    else if(output_active[i] == false && gwizd_on == false) // outpin == false
    {
      digitalWriteFast(outpin[i], HIGH);  // wylaczamy przekaznik OFF
    }
  }
}


// POBIERA ADRES ZE ZWOREK I USTAWIA GO DLA RFki
void Odbiornik::setRFaddress()
{
  if(     addr1_State == false && addr2_State == false && addr3_State == false){address_nr = 0;}
  else if(addr1_State == false && addr2_State == false && addr3_State == true){address_nr = 1;}
  else if(addr1_State == false && addr2_State == true && addr3_State == false){address_nr = 2;}
  else if(addr1_State == false && addr2_State == true && addr3_State == true){address_nr = 3;}
  else if(addr1_State == true && addr2_State == false && addr3_State == false){address_nr = 4;}
  else if(addr1_State == true && addr2_State == false && addr3_State == true){address_nr = 5;}
  else if(addr1_State == true && addr2_State == true && addr3_State == false){address_nr = 6;}
  else if(addr1_State == true && addr2_State == true && addr3_State == true){address_nr = 7;}

  radio.begin();
  radio.openReadingPipe(1, address[address_nr]);
  //radio.enableAckPayload();
  //radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(95);
  //radio.setRetries(5,15); // delay, count
  //radio.setPayloadSize();
  radio.startListening();
}


// SPRAWDZA CZY NASTAPILA ZMIANA W ZWORKACH
// JESLI TAK TO USTAWIA NOWY ADRES DLA ODBIORNIKA
void Odbiornik::manage_zworki()
{
  // ODCZYTUJEMY WARTOSCI ZE ZWOREK (ZW1 - ZW3) W URZADZENIU
  addr1_State = !digitalReadFast(ADDR1);
  addr2_State = !digitalReadFast(ADDR2);
  addr3_State = !digitalReadFast(ADDR3);

  if( addr1_State != prev_addr1_State || addr2_State != prev_addr2_State || addr3_State != prev_addr3_State )
  {
    prev_addr1_State = addr1_State;
    prev_addr2_State = addr2_State;
    prev_addr3_State = addr3_State;
    setRFaddress();
  }
}

class NRFka : public Odbiornik
{
  private:
    RF24 radyjo;
  public:
    void set();
};

void NRFka::set()
{
  
}

