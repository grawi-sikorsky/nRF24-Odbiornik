#include "outputs.h"
#include "configuration.h"

void Outputs::setupOutputs(){
    #ifdef DEBUGSERIAL
        relays[0].setRelayNumber( OUTPIN0 );
        relays[1].setRelayNumber( OUTPIN1 );
        relays[2].setRelayNumber( OUTPIN2 );
        relays[3].setRelayNumber( OUTPIN3 );
        relays[4].setRelayNumber( OUTPIN6 );
        relays[5].setRelayNumber( OUTPIN7 );
    #else
        relays[0].setRelayNumber( OUTPIN0 );
        relays[1].setRelayNumber( OUTPIN1 );
        relays[2].setRelayNumber( OUTPIN2 );
        relays[3].setRelayNumber( OUTPIN3 );
        relays[4].setRelayNumber( OUTPIN4 );
        relays[5].setRelayNumber( OUTPIN5 );
        relays[6].setRelayNumber( OUTPIN6 );
        relays[7].setRelayNumber( OUTPIN7 );
    #endif
}

void Outputs::disableAllOutputs(){
    for (int i = 0; i < RELAYS_COUNT; i++){
        relays[i].deactivate();
    }
}

void Outputs::manageTimeouts(){
    for (int i = 0; i < RELAYS_COUNT; i++){
        if(relays[i].isTimeout() && relays[i].getIsActive()){
            relays[i].deactivate();
        }
    }
}
void Outputs::manageBlinks(){
    for (int i = 0; i < RELAYS_COUNT; i++){
        if(relays[i].getLightType() == Blink && relays[i].getIsActive())
        {
            if(millis() - relays[i].getLastBlinkTime() >= relays[i].getBlinkTimeMS())
            {
                digitalWriteFast(LEDPIN, !digitalReadFast(LEDPIN)); // odwroc stan 
                digitalWriteFast(relays[i].getRelayNumber(), !digitalReadFast(relays[i].getRelayNumber())); // odwracamy przekaznik

                relays[i].setLastBlinkTime( millis() );
            }
            else{

            }
        }
    }
}