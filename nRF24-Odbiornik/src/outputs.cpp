#include "outputs.h"
#include "configuration.h"

void Outputs::setupOutputs(){
    #ifdef DEBUGSERIAL
        relays[0].setRelayPin( OUTPIN0 );
        relays[1].setRelayPin( OUTPIN1 );
        relays[2].setRelayPin( OUTPIN2 );
        relays[3].setRelayPin( OUTPIN3 );
        relays[4].setRelayPin( OUTPIN6 );
        relays[5].setRelayPin( OUTPIN7 );
    #else
        relays[0].setRelayPin( OUTPIN0 );
        relays[1].setRelayPin( OUTPIN1 );
        relays[2].setRelayPin( OUTPIN2 );
        relays[3].setRelayPin( OUTPIN3 );
        relays[4].setRelayPin( OUTPIN4 );
        relays[5].setRelayPin( OUTPIN5 );
        relays[6].setRelayPin( OUTPIN6 );
        relays[7].setRelayPin( OUTPIN7 );
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
                digitalWriteFast(relays[i].getRelayPin(), !digitalReadFast(relays[i].getRelayPin())); // odwracamy przekaznik

                relays[i].setLastBlinkTime( millis() );
            }
            else{

            }
        }
    }
}