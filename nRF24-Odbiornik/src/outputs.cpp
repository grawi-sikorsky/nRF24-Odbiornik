#include "outputs.h"
#include "configuration.h"

void Outputs::setupOutputs(){
    #ifdef DEBUGSERIAL
        output_size = 6;
        relays[0].setRelayNumber( OUTPIN0 );
        relays[1].setRelayNumber( OUTPIN1 );
        relays[2].setRelayNumber( OUTPIN2 );
        relays[3].setRelayNumber( OUTPIN3 );
        relays[4].setRelayNumber( OUTPIN6 );
        relays[5].setRelayNumber( OUTPIN7 );
    #else
        output_size = 8;
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
    for (int i = 0; i < output_size; i++){
        relays[i].deactivate();
    }
}

void Outputs::manageTimeouts(){
    for (int i = 0; i < output_size; i++){
        if(relays[i].isTimeout() && relays[i].getIsActive()){
            relays[i].deactivate();
        }
    }
}
void Outputs::manageBlinks(){
    for (int i = 0; i < output_size; i++){
        if(relays[i].getLightType() == Blink && relays[i].getIsActive())
        {
            if(millis() - relays[i].getBlinkTime() >= relays[i].getBlinkTimeMS())
            {
                digitalWriteFast(LEDPIN, !digitalReadFast(LEDPIN)); // odwroc stan 
                digitalWriteFast(relays[i].getRelayNumber(), !digitalReadFast(relays[i].getRelayNumber())); // odwracamy przekaznik

                relays[i].setBlinkTime( millis() );
            }
            else{

            }
        }
    }
}