#include "../src/odbiornik.h"
#include "../src/outputs.h"

extern RF24 radio; // CE, CSN
extern WhistleData whistleData;

Outputs outputs;


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

  outputs.setupOutputs();
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

void Odbiornik::setLEDstate(bool state)
{
  digitalWriteFast(LEDPIN, state);
}

bool Odbiornik::getLEDstate()
{
  return digitalReadFast(LEDPIN);
}

void Odbiornik::manageLed(){
  if(millis() - LEDlastActivationTime >= INFO_LED_TIME){
    this->setLEDstate(false);
  }
}

void Odbiornik::setLedActive(){
  LEDlastActivationTime = millis();
}

bool Odbiornik::isWhistleSignal(){
  if(whistleData.getgwizd == 1){
    return true;
  }
  return false;
}

bool Odbiornik::isHelperSignal()
{
  if( whistleData.getgwizd == 11 || whistleData.getgwizd == 12 || whistleData.getgwizd == 13 ||
      whistleData.getgwizd == 21 || whistleData.getgwizd == 22 || whistleData.getgwizd == 23 )
    {
      return true;
    }
  return false;
}

bool Odbiornik::isPhysicalSignal()
{
  inPin1_State = digitalReadFast(INPIN1);
  if( inPin1_State != inPin1_prev_State ) return true;  // jesli na ktorymkolwiek pinie wystapia zmiana zwroc TRUE
  else return false;                                    // jesli bez zmian -> FALSE
}

void Odbiornik::manageInputWireless()
{
  if(isWhistleSignal())
  {
    outputs.relays[0].activate(2000, ElightType::Solid, 0);
    outputs.relays[1].activate(3000, ElightType::Solid, 0);
    outputs.relays[2].activate(4000, ElightType::Solid, 0);
    outputs.relays[3].activate(5000, ElightType::Blink, 400, 0);
    outputs.relays[4].activate(2000, ElightType::Blink, 200, 0);
    outputs.relays[5].activate(6000, ElightType::Blink, 500, 0);
  }
  
  if (isHelperSignal() == true )
  {
    //
    whistleData.getgwizd = 2; // default state...
  }
}

void Odbiornik::manageInputPhysical()
{
  if (isPhysicalSignal())
  {
    if( inPin1_State != inPin1_prev_State )
    {
      if(inPin1_State == LOW)
      {
        outputs.relays[5].activate(2000, ElightType::Solid, Eevoker::Physical);
      }
      inPin1_prev_State = inPin1_State;
    }
  }
}

void Odbiornik::manageOutputs()
{
  outputs.manageBlinks();
  outputs.manageTimeouts();
}

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

void Odbiornik::manageZworki()
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