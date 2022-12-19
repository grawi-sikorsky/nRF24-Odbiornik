class Relay
{
private:
    int activeTimeMS = 0;
    bool isActive = false;

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

public:
    void setActiveTimeMS(int timeMS) { this->activeTimeMS = timeMS; };
    void setEvoker(int evo) { this->evoker = evo; };
    void setLightType(int lightType) { this->lightType = lightType; };
    int getActiveTimeMS() { return this->activeTimeMS; };
    int getEvoker() { return this->evoker; };
    int getLightType() { return this->lightType; };
    
    void activate(int, int, int);
    void deactivate();
};