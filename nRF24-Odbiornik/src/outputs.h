#include "relay.h"

class Outputs
{

public:
    Relay relays[6];

    void setupOutputs();
    void disableAllOutputs();
    void manageTimeouts();

};