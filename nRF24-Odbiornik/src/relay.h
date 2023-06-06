#include <time.h>
#include "configuration.h"
#include "relaySettingStruct.h"

enum ElightType
{
    Solid,
    Blink,
    FastBlink,
    SlowBlink
};

enum Eevoker
{
    Whistle,
    WhistleButton,
    Helper,
    Physical
};

class Relay
{
private:
    uint16_t activeTimeMS = 0;
    time_t activateTime;
    uint16_t blinkTimeMS = 0;
    time_t lastBlinkTime;

    bool isActive = false;
    uint8_t lightType;
    uint8_t evoker;
    uint8_t relayPin = 0;

    // GETTERS SETTERS
    uint16_t getActiveTimeMS() const { return activeTimeMS; }
    void setActiveTimeMS(uint16_t activeTimeMS_) { activeTimeMS = activeTimeMS_; }

    void setIsActive(bool isActive_) { isActive = isActive_; }

    time_t getActivateTime() const { return activateTime; }
    void setActivateTime(const time_t &activateTime_) { activateTime = activateTime_; }

    void setLightType(uint8_t lightType_) { lightType = lightType_; }

    int getEvoker() const { return evoker; }
    void setEvoker(uint8_t evoker_) { evoker = evoker_; }

public:
    void setRelayPin(uint8_t relayPin_) { relayPin = relayPin_; }
    bool getIsActive() const { return isActive; }
    int getLightType() const { return lightType; }

    uint16_t getBlinkTimeMS() const { return blinkTimeMS; }
    void setBlinkTimeMS(uint16_t blinkTimeMS_) { blinkTimeMS = blinkTimeMS_; }

    time_t getLastBlinkTime() const { return lastBlinkTime; }
    void setLastBlinkTime(const time_t &lastBlinkTime_) { lastBlinkTime = lastBlinkTime_; }

    uint8_t getRelayPin() const { return relayPin; }

    void activate(uint16_t timeMS, uint8_t lightType, uint8_t evoker);
    void activate(uint16_t timeMS, uint8_t lightType, uint16_t blinkTime, uint8_t evoker);
    void activate(RelaySetting setting);
    void deactivate();
    bool isTimeout();
};