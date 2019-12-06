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
            int fuel = msg.fuel;
            int time_to_runway =  msg.eta;
            if(msg.priority=1){
                //É prioritária
            }
        }
        if(msg.mtype == DEPARTURE){
            int takeoff_instant =  msg.takeoff;


        }
    }
}

