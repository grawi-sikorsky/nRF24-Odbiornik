#include <time.h>

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
    Helper,
    Physical
};

class Relay
{
private:
    int activeTimeMS = 0;
    time_t activateTime;
    int blinkTimeMS = 0;
    time_t lastBlinkTime;

    bool isActive = false;
    int lightType;
    int evoker;
    int relayNumber = 0;

    void changePinState();

    // GETTERS SETTERS
    int getActiveTimeMS() const { return activeTimeMS; }
    void setActiveTimeMS(int activeTimeMS_) { activeTimeMS = activeTimeMS_; }

    void setIsActive(bool isActive_) { isActive = isActive_; }

    time_t getActivateTime() const { return activateTime; }
    void setActivateTime(const time_t &activateTime_) { activateTime = activateTime_; }

    void setLightType(int lightType_) { lightType = lightType_; }

    int getEvoker() const { return evoker; }
    void setEvoker(int evoker_) { evoker = evoker_; }



public:
    void setRelayNumber(int relayNumber_) { relayNumber = relayNumber_; }
    bool getIsActive() const { return isActive; }
    int getLightType() const { return lightType; }

    int getBlinkTimeMS() const { return blinkTimeMS; }
    void setBlinkTimeMS(int blinkTimeMS_) { blinkTimeMS = blinkTimeMS_; }

    time_t getLastBlinkTime() const { return lastBlinkTime; }
    void setLastBlinkTime(const time_t &lastBlinkTime_) { lastBlinkTime = lastBlinkTime_; }

    int getRelayNumber() const { return relayNumber; }


    void activate(int timeMS, int lightType, int evoker);
    void activate(int timeMS, int lightType, int blinkTime, int evoker);
    void deactivate();
    bool isTimeout();
};