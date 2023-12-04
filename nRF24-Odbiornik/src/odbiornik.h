#include <Arduino.h>
#include "configuration.h"
#include "relaySettingStruct.h"
#include "pairingDataStruct.h"
#include <EEPROM.h>

enum EWhistleCommands
{
	ELightsOn,
	ETimerStop,
	EDefaultState
};

enum EDevices
{
	EWhistle,
	EController,
};

struct WhistleData
{
	uint8_t device = EWhistle;
	uint8_t command = EDefaultState;
};

class Odbiornik
{
private:
	time_t LEDlastActivationTime;

	int address_nr = 0;		 // wybor adresu z tablicy powyzej
	bool addr_State[3];		 //, addr2_State, addr3_State,
	bool prev_addr_State[3]; //, prev_addr2_State, prev_addr3_State;
	bool inPin1_State, inPin1_prev_State;
	bool isSettingsMode;

public:
	// Ustawia piny, pinmode,
	void init();

	// inizjalizuje nrfke
	void initRF();

	// SET INFO LED ON or OFF
	void setLEDstate(bool state);
	bool getLEDstate();
	void manageLed();
	void setLedActive();
	bool isInSettingsMode();

	bool isWhistleSignal();

	bool isWhistleButtonSignal();

	// Sprawdza czy w danych z RF pojawily sie wartosci 11 12 13 21 22 23.
	bool isHelperSignal();

	// Sprawdza czy stan wejsc fizycznych w odbiorniku sie zmienil
	bool isPhysicalSignal();

	// Obsluga wejsc radiowych
	void updateInputWireless();

	void updateInputWirelessV2();

	void activateRelaysByEvoker(uint8_t evoker);

	// Obsluga wejsc fizycznych w odbiorniku
	void updateInputPhysical();

	void updateOutputs();

	// POBIERA ADRES ZE ZWOREK I USTAWIA GO DLA RFki
	void setRFaddress();

	// SPRAWDZA CZY NASTAPILA ZMIANA W ZWORKACH
	// JESLI TAK TO USTAWIA NOWY ADRES DLA ODBIORNIKA
	void updateJumpers();

	void initializeEEPROM();
	void saveSettings(RelaySetting settings[]);
	void readSettings(RelaySetting settings[]);
	void printRelayEepromSettings();

	void processSettings();


	
};