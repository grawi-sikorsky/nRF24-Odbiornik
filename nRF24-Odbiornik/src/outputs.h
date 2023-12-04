#include "relay.h"

class Outputs
{
    public:
        Relay relays[RELAYS_COUNT];

        void setupOutputs();
        void disableAllOutputs();
        void updateTimeouts();
        void updateBlinks();
};