#include "relay.h"

class Outputs
{

public:
    Relay relays[7];

    void setupOutputs();
    void disableAllOutputs();
    void manageTimeouts();

};