#include "shared.h"

void control_tower(){
    control_tower_msg msg;
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, show_stats);
    logger("Control Tower - Central Process Created\n");
    while(!TERMINATE){
        if(msgrcv(msqid,&msg,sizeof(msg),0,0) < 0){
            logger("Error receiving messagem from message queue.\n");
            exit(0); 
        }
        if(msg.mtype == ARRIVAL){
            printf("fuel: %d\neta: %d\n", msg.fuel, msg.eta);
        }
        if(msg.mtype == DEPARTURE){
            printf("takeoff: %d\n", msg.takeoff);
        }
    }
}

