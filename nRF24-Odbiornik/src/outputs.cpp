#include "outputs.h"
#include "configuration.h"

void Outputs::setupOutputs(){
    relays[0].setRelayNumber( OUTPIN0 );
    relays[1].setRelayNumber( OUTPIN1 );
    relays[2].setRelayNumber( OUTPIN2 );
    relays[3].setRelayNumber( OUTPIN3 );
    relays[4].setRelayNumber( OUTPIN6 );
    relays[5].setRelayNumber( OUTPIN7 );
}

void Outputs::disableAllOutputs(){
    for (int i = 0; i<6; i++){
        relays[i].deactivate();
    }
}

void Outputs::manageTimeouts(){
    for (int i = 0; i<6; i++){
        if(relays[i].isTimeout() && relays[i].getIsActive()){
            relays[i].deactivate();
        }
    }
}