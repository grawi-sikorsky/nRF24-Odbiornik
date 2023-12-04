#include "../src/odbiornik.h"
#include "../src/outputs.h"
#include "util.h"

extern RF24 radio; // CE, CSN
extern WhistleData whistleData;
extern RelaySetting relaySetting;
extern PairingData pairingData;
RelaySetting relaySettings[8];

Outputs outputs;
uint8_t address[][6] = { "1Node", "2Node", "3Node", "4Node", "5Node", "6Node" };

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
    #ifndef DEBUG_STUB
      pinModeFast(OUTPIN4, OUTPUT); // RX
      pinModeFast(OUTPIN5, OUTPUT); // RX
    #endif
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

  initializeEEPROM();

  outputs.setupOutputs();
  #ifdef DEBUGRELAYS
    printRelayEepromSettings();
  #endif
}

void Odbiornik::initRF()
{
  radio.begin();
  radio.openReadingPipe(EWhistle, address[0]);
  radio.openReadingPipe(EController, address[1]);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(95);
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

bool Odbiornik::isInSettingsMode(){
  return isSettingsMode;
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
  if(whistleData.command == ELightsOn){
    return true;
  }
  return false;
}

bool Odbiornik::isWhistleButtonSignal(){
  if(whistleData.command == ETimerStop){
    return true;
  }
  return false;
}

bool Odbiornik::isHelperSignal()
{
  if( whistleData.command == 11 || whistleData.command == 12 || whistleData.command == 13 ||
      whistleData.command == 21 || whistleData.command == 22 || whistleData.command == 23 )
    {
      return true;
    }
  return false;
}

bool Odbiornik::isPhysicalSignal()
{
  inPin1_State = digitalReadFast(INPIN1);
  if( inPin1_State != inPin1_prev_State ) return true;
  else return false;
}

void Odbiornik::processSettings(){
  if(relaySetting.relayNumber >= 0 && relaySetting.relayNumber < 8){

    int number = relaySetting.relayNumber;

    relaySetting.relayTime = ntohs(relaySetting.relayTime);
    relaySetting.relayBlinkTime = ntohs(relaySetting.relayBlinkTime);

    relaySettings[number].relayEnabled = relaySetting.relayEnabled;
    relaySettings[number].relayType = relaySetting.relayType;
    relaySettings[number].relayTime = relaySetting.relayTime;
    relaySettings[number].relayBlinkTime = relaySetting.relayBlinkTime;
    relaySettings[number].relayEvoker = relaySetting.relayEvoker;
    relaySettings[number].relayReceiverNumber = relaySetting.relayReceiverNumber;
  }

  #ifdef DEBUGRELAYS
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayEnabled: ")); Serial.println(relaySetting.relayEnabled);
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayNumber: ")); Serial.println(relaySetting.relayNumber);
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayType: ")); Serial.println(relaySetting.relayType);
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayTime: ")); Serial.println(relaySetting.relayTime);
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayBlinkTime: ")); Serial.println(relaySetting.relayBlinkTime);
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayEvoker: ")); Serial.println(relaySetting.relayEvoker);
    Serial.print(relaySetting.relayNumber); Serial.print(F(" relayReceiverNumber: ")); Serial.println(relaySetting.relayReceiverNumber);
    Serial.println("============================================================");
  #endif
}

void Odbiornik::updateInputWireless()
{
  if(isWhistleSignal())
  {
    outputs.relays[0].activate(relaySettings[0].relayTime,relaySettings[0].relayType,0);
    outputs.relays[1].activate(relaySettings[1].relayTime,relaySettings[1].relayType,0);
    outputs.relays[2].activate(relaySettings[2].relayTime,relaySettings[2].relayType,0);
    outputs.relays[3].activate(relaySettings[3].relayTime,relaySettings[3].relayType,0);
    // outputs.relays[4].activate(relaySettings[4].relayTime,relaySettings[4].relayType,0);
    // outputs.relays[5].activate(relaySettings[5].relayTime,relaySettings[5].relayType,0);
  }

  if(isWhistleButtonSignal()){
    outputs.relays[5].activate(relaySettings[5].relayTime,relaySettings[5].relayType,0);
  }
  
  if (isHelperSignal())
  {
    outputs.relays[4].activate(2000, ElightType::Solid, Eevoker::Helper);
    whistleData.command = EDefaultState; // default state...
  }
}

void Odbiornik::updateInputWirelessV2(){
  if(isWhistleSignal())
  {
    activateRelaysByEvoker(Eevoker::Whistle);
  }

  if(isWhistleButtonSignal()){
    activateRelaysByEvoker(Eevoker::WhistleButton);
  }
  
  if (isHelperSignal())
  {
    activateRelaysByEvoker(Eevoker::Helper);
  }
}

void Odbiornik::updateInputPhysical()
{
  if (isPhysicalSignal())
  {
    activateRelaysByEvoker(Eevoker::Physical);
  }
}

void Odbiornik::activateRelaysByEvoker(uint8_t evoker){
  for (int i = 0; i < RELAYS_COUNT; i++){
    if( relaySettings[i].relayEvoker == evoker && relaySettings[i].relayEnabled == true){
      if(outputs.relays[i].getIsActive() == false){
        outputs.relays[i].activate(relaySettings[i]);
      }
    }
  }
}

void Odbiornik::updateOutputs()
{
  outputs.updateBlinks();
  outputs.updateTimeouts();
}

void Odbiornik::setRFaddress()
{
  isSettingsMode = false;

  if(     addr_State[0] == false  && addr_State[1] == false   && addr_State[2] == false)  {address_nr = 0;}
  else if(addr_State[0] == false  && addr_State[1] == false   && addr_State[2] == true)   {address_nr = 1;}
  else if(addr_State[0] == false  && addr_State[1] == true    && addr_State[2] == false)  {address_nr = 2;}
  else if(addr_State[0] == false  && addr_State[1] == true    && addr_State[2] == true)   {address_nr = 3;}
  else if(addr_State[0] == true   && addr_State[1] == false   && addr_State[2] == false)  {address_nr = 4;}
  else if(addr_State[0] == true   && addr_State[1] == false   && addr_State[2] == true)   {address_nr = 5;}
  else if(addr_State[0] == true   && addr_State[1] == true    && addr_State[2] == false)  {address_nr = 6;}
  else if(addr_State[0] == true   && addr_State[1] == true    && addr_State[2] == true)   {isSettingsMode = true;}

  radio.begin();
  radio.openReadingPipe(1, address[0]);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(95);
  radio.startListening();
}

void Odbiornik::updateJumpers()
{
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

void Odbiornik::saveSettings(RelaySetting settings[]){
  int addr = 0;  // Starting address in EEPROM
  for (int i = 0; i < RELAYS_COUNT; i++) {
    EEPROM.put(addr, settings[i]);
    addr += sizeof(RelaySetting);  // Increment address by the size of each item
  }
  #ifdef DEBUGSERIAL
    Serial.println(F("Settings Saved.."));
  #endif
}

void Odbiornik::readSettings(RelaySetting settings[]){
  int addr = 0;  // Starting address in EEPROM
  for (int i = 0; i < RELAYS_COUNT; i++) {
    EEPROM.get(addr, settings[i]);
    addr += sizeof(RelaySetting);  // Increment address by the size of each item
  }
  #ifdef DEBUGSERIAL
    Serial.println(F("Settings Readed.."));
  #endif
}

void Odbiornik::printRelayEepromSettings(){
  for (int i = 0; i < RELAYS_COUNT; i++) {
    #ifdef DEBUGSERIAL
      Serial.print(F("relayEnabled: ")); Serial.println(relaySettings[i].relayEnabled);
      Serial.print(F("relayNumber: ")); Serial.println(relaySettings[i].relayNumber);
      Serial.print(F("relayType: ")); Serial.println(relaySettings[i].relayType);
      Serial.print(F("relayTime: ")); Serial.println(relaySettings[i].relayTime);
      Serial.print(F("relayBlinkTime: ")); Serial.println(relaySettings[i].relayBlinkTime);
      Serial.print(F("relayEvoker: ")); Serial.println(relaySettings[i].relayEvoker);
    #endif
  }
}

void Odbiornik::initializeEEPROM(){
  // not touched eeprom have always HIGH value so bool becomes 1 = true
  bool isEepromNotInitialized = EEPROM.read(EEPROM_INIT_PLACE);
  #ifdef DEBUGSERIAL
    Serial.print(F("Is EEPROM init needed? : ")); Serial.println(isEepromNotInitialized);
  #endif

  if(isEepromNotInitialized){
    #ifdef DEBUGSERIAL
      Serial.println(F("Initialize EEPROM..."));
    #endif
    RelaySetting DEFAULT_SETTINGS[] = {
      DEFAULT_RELAY_1,
      DEFAULT_RELAY_2,
      DEFAULT_RELAY_3,
      DEFAULT_RELAY_4,
      DEFAULT_RELAY_5,
      DEFAULT_RELAY_6,
      DEFAULT_RELAY_7,
      DEFAULT_RELAY_8
    };
    saveSettings(DEFAULT_SETTINGS);
    EEPROM.put(EEPROM_INIT_PLACE, false);
  }

  readSettings(relaySettings);
}