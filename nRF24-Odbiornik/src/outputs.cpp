#include "outputs.h"

void Outputs::disableAllOutputs(){
    for (int i = 0; i<8; i++){
        relays[i].deactivate();
    }
    
}