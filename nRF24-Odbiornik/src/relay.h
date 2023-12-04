#include <time.h>
#include "configuration.h"
#include "relaySettingStruct.h"

enum ElightType
{
    Solid,
    Blink
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
    uint16_t duration = 0;
    time_t activationTime;
    uint16_t blinkDuration = 0;
    time_t lastBlinkTime;

    bool isActive = false;
    uint8_t lightType;
    uint8_t evoker;
    uint8_t relayPin = 0;

    // GETTERS SETTERS
    uint16_t getDuration() const { return duration; }
    void setDuration(uint16_t activeTimeMS_) { duration = activeTimeMS_; }

    time_t getActivationTime() const { return activationTime; }
    void setActivationTime(const time_t &activationTime_) { activationTime = activationTime_; }

    void setLightType(uint8_t lightType_) { lightType = lightType_; }

    int getEvoker() const { return evoker; }
    void setEvoker(uint8_t evoker_) { evoker = evoker_; }

public:
    void setRelayPin(uint8_t relayPin_) { relayPin = relayPin_; }
    uint8_t getRelayPin() const { return relayPin; }
    
    int getLightType() const { return lightType; }

    void setIsActive(bool isActive_) { isActive = isActive_; }
    bool getIsActive() const { return isActive; }

    uint16_t getBlinkDuration() const { return blinkDuration; }
    void setBlinkDuration(uint16_t blinkDuration_) { blinkDuration = blinkDuration_; }

    time_t getLastBlinkTime() const { return lastBlinkTime; }
    void setLastBlinkTime(const time_t &lastBlinkTime_) { lastBlinkTime = lastBlinkTime_; }

    void activate(uint16_t timeMS, uint8_t lightType, uint8_t evoker);
    void activate(uint16_t timeMS, uint8_t lightType, uint16_t blinkTime, uint8_t evoker);
    void activate(RelaySetting setting);
    void deactivate();
    bool isTimeout();
};