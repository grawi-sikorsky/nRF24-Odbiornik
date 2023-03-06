#include "relay.h"

class Outputs
{

    public:
        int output_size;
        Relay relays[6];

        void setupOutputs();
        void disableAllOutputs();
        void manageTimeouts();
        void manageBlinks();

};