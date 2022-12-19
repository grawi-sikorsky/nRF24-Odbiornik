#include <time.h>

class Relay
{
private:
    int activeTimeMS = 0;
    bool isActive = false;
    time_t activateTime;

    enum ElightType
    {
        Solid,
        Blink,
        FastBlink,
        SlowBlink
    };
    enum Eevoker
    {
        Gwizdek,
        Pomocniczy,
        Fizyczne
    };
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

    int getLightType() const { return lightType; }
    void setLightType(int lightType_) { lightType = lightType_; }

    int getEvoker() const { return evoker; }
    void setEvoker(int evoker_) { evoker = evoker_; }

    int getRelayNumber() const { return relayNumber; }


public:

    void setRelayNumber(int relayNumber_) { relayNumber = relayNumber_; }
    bool getIsActive() const { return isActive; }

    void activate(int timeMS, int lightType, int evoker);
    void deactivate();
    bool isTimeout();

};