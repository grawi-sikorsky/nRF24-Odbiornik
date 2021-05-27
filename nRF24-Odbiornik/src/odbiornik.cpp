#include "../src/odbiornik.h"

extern RF24 radio; // CE, CSN
extern outdata nrfdata;

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

  for(int i=0; i<outpin_array_len; ++i) // zerujemy
  {
    output_strobo[i] = false;
    output_active[i] = false;
  }

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

bool Odbiornik::getInfoLED()
{
  return digitalReadFast(LEDPIN);
}

void Odbiornik::check_whistle()
{

}


// USTAWIA OUTPUTY W JEDNEJ FUNKCJI
// PRZYJMUJE NUMER WYJSCIA KTORY CHCEMY AKTYWOWAC [ 0 - 7 ]
// WIEKSZA WARTOSC ZOSTAJE POMINIETA
void Odbiornik::set_output(Zrodlo source, bool state, int pin=10 )
{
  if( source == EGwizdek)
  {
    input_source[pin] = false;
  }
  else if (source == EPomocniczy || source == EFizyczne)
  {
    input_source[pin] = true;
  }
  output_active[pin] = state;
  prevOutputTime[pin] = millis();
}

void Odbiornik::set_output_strobo(Zrodlo source, bool state, int pin=10)
{
  if( source == EGwizdek)
  {
    input_source[pin] = false;
  }
  else if (source == EPomocniczy || source == EFizyczne)
  {
    input_source[pin] = true;
  }
  output_strobo[pin] = state;
  //output_active[pin] = state; //???????? chyba nie trzeba?
  prevOutputTime[pin] = millis();
}


bool Odbiornik::read_input_gwizdek()
{
  if( nrfdata.getgwizd == 0 || nrfdata.getgwizd == 1 || nrfdata.getgwizd == 2 )
  {
    return true;
  }
  return false;
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
  if( inPin1_State != inPin1_prev_State ) return true;  // jesli na ktorymkolwiek pinie wystapia zmiana zwroc TRUE
  else return false;                                    // jesli bez zmian -> FALSE
}

// Obsluga wejsc INPUT i przypisanie odpowiednim WYJSCIOM flagi aktywnej
// Obsluga wejsc z nadajnika pomocniczego RF
// Tutaj jedynie ustawiamy co ma zostac wlaczone -> czesc wykonawcza w manage_output()
// konfiguracja:
// KOSZ LEWY POM && KOSZ LEWY CZER = KOSZ LEWY LED && PODLOGA LEWY LED
// KOSZ PRAWY POM && KOSZ PRAWY CZER = KOSZ PRAWY LED && PODLOGA PRAWY LED
void Odbiornik::manage_input_rf()
{
  // 1. Obsluga gwizdka
  if( read_input_gwizdek() == true )
  {
    if(nrfdata.getgwizd == 1)                 // JESLI GWIZDEK WYSLAL SYGNAL O WZROSCIE CISNIENIA
    {
      gwizdTimeout_start_at = millis();                        // ustaw czas ostatniego gwizdniecia

      //output_active[0] = true;                          // Obecnie klient potrzebuje wylacznie jednego przekaznika
      set_output(EGwizdek, true, 0);
      set_output_strobo(EGwizdek, true, 1);
    }
    // Uzywane tylko gdy gwizdek jest w trybie wysylania takze sygnalu wylaczajacego (domyslnie wysyla tylko wlaczenie sygnalu)
    else if(nrfdata.getgwizd == 0)    // JESLI GWIZDEK WYSLAL SYGNAL O SPADKU CISNIENIA - WYLACZAMY PRZEKAZNIKI
    {
      set_output(EGwizdek, false, 0);
      set_output_strobo(EGwizdek, false, 1);
    }
    else if(nrfdata.getgwizd == 2)  // transmisja off : J.W. dotyczy drugiej metody
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
  
  // 2. Obsluga ndajnika pomocniczego
  if (read_input_rf() == true )
  {
    #if defined (TEST_MODE)
      #ifdef DEBUGSERIAL
        set_output(0,1,2,3,4,5);  // JESLI TESTMODE (EWRYFINK IS ILUMINEJTED!)
      #else
        set_output(0,1,2,3,4,5,6,7);  // JESLI TESTMODE (EWRYFINK IS ILUMINEJTED!)
      #endif
    #elif defined (EASY_MODE)
      set_output(EPomocniczy, true, 0);
      set_output_strobo(EPomocniczy, true, 1);
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

// OBSLUGA WYLACZNIE WEJSC FIZYCZNYCH ODBIORNIKA
void Odbiornik::manage_input_odb()
{
  // 3. Obsluga wejsc fizycznych odbiornika
  if (read_input_pins() == true )  // jesli byla zmiana na wejsciach sprawdza kazde po kolei i przypisuje konkretne wyjscia 
  {
    // INPUT PIN 1 - 
    if( inPin1_State != inPin1_prev_State )                   // jesli nastapila zmiana:
    {
      if(inPin1_State == LOW)                                 // i byla to zmiana na LOW czyli aktywne
      {
        set_output( EFizyczne, true, 0 );
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
    if(output_active[i] == true && output_strobo[i] == false) // wyglada na to ze zawsze true bez wzgledu na drugi parametr... TODO!
    {
      digitalWriteFast(outpin[i], LOW); // uruchamiamy przekaznik ON

      // SPRAWDZA CZY CZAS ZOSTAL PRZEKROCZONY
      // JESLI TAK WYLACZA WYJSCIE
      // JESLI NIE - NYC
      if((outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME) && input_source[i] == true) // DLA WEJSC FIZYCZNYCH I POMOCNICZEGO
      {
        prevOutputTime[i] = outputCurrentTime; // zeruj licznik
        output_active[i] = false; // flaga output na false
        input_source[i] = false; // info o wejsciu
        nrfdata.getgwizd=0; // testowo
      }

      if((outputCurrentTime - gwizdTimeout_start_at >= OUTPUT_GWIZD_TIME) && input_source[i] == false)
      {
        gwizdTimeout_start_at = outputCurrentTime;  // zeruj licznik
        output_active[i] = false;                   // flaga output na false
        input_source[i] = false;                    // info o wejsciu
        nrfdata.getgwizd=0; // testowo
      }
    }
    else if(output_strobo[i] == true) // flaga strobo powinna wystarczyc? TODO
    {

      if(outputCurrentTime - outputStroboTime[i] >= STROBO_FREQ)
      {
        digitalWriteFast(LEDPIN, !digitalReadFast(LEDPIN)); // odwroc stan 
        output_active[i] = false; // ????????
        digitalWriteFast(outpin[i], !digitalReadFast(outpin[i])); // odwracamy przekaznik
        outputStroboTime[i] = outputCurrentTime;
      }
      if(outputCurrentTime - prevOutputTime[i] >= OUTPUT_TIME)
      {
        output_strobo[i] = false; // wylaczamy strobo
        nrfdata.getgwizd=0;// testowo
      }
    }
    else if(output_active[i] == false || output_strobo[i] == false) // outpin == false
    {
      digitalWriteFast(outpin[i], HIGH);  // wylaczamy przekaznik OFF
    }
    else if(output_active[i]==false && output_strobo[i]==false)
    {
      nrfdata.getgwizd=0; // na wszelki wypadek gdyby nadajnik byl poza zasiegiem niet..
    }
  }
}


// POBIERA ADRES ZE ZWOREK I USTAWIA GO DLA RFki
void Odbiornik::setRFaddress()
{
  if(     addr_State[0] == false  && addr_State[1] == false   && addr_State[2] == false)  {address_nr = 0;}
  else if(addr_State[0] == false  && addr_State[1] == false   && addr_State[2] == true)   {address_nr = 1;}
  else if(addr_State[0] == false  && addr_State[1] == true    && addr_State[2] == false)  {address_nr = 2;}
  else if(addr_State[0] == false  && addr_State[1] == true    && addr_State[2] == true)   {address_nr = 3;}
  else if(addr_State[0] == true   && addr_State[1] == false   && addr_State[2] == false)  {address_nr = 4;}
  else if(addr_State[0] == true   && addr_State[1] == false   && addr_State[2] == true)   {address_nr = 5;}
  else if(addr_State[0] == true   && addr_State[1] == true    && addr_State[2] == false)  {address_nr = 6;}
  else if(addr_State[0] == true   && addr_State[1] == true    && addr_State[2] == true)   {address_nr = 7;}

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
  addr_State[0] = !digitalReadFast(ADDR1);
  addr_State[1] = !digitalReadFast(ADDR2);
  addr_State[2] = !digitalReadFast(ADDR3);

  for(int i=0; i<3; i++)
  {
    if( addr_State[i] != prev_addr_State[i])
    {
      prev_addr_State[i] = addr_State[i];
      setRFaddress();
    }
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

