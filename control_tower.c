#include "shared.h"



void control_tower(){
    
    signal(SIGUSR1, show_stats);
    logger("Control Tower - Central Process Created\n");
    while(!TERMINATE){
    }

}

