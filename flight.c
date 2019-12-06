#include "shared.h"



void * flight_arrival(void*arg){
    pthread_mutex_lock(&flight_arrival_mutex);
    flight_arrival_t * arrival = (flight_arrival_t*)arg;
    control_tower_msg msg;
    msg.mtype = ARRIVAL;
    msg.eta = arrival->eta;
    msg.fuel = arrival->fuel;
    if(arrival->fuel <= 4+(arrival->eta)+settings.landing_duration){
        msg.priority = 1;
    }
    msg.priority = 0;
    if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
        printf("Error sending message through message queue (%s).\n",strerror(errno));
        exit(0);
    }
    pthread_mutex_unlock(&flight_arrival_mutex);
    return 0;
}

void * flight_departure(void*arg){
    pthread_mutex_lock(&flight_departure_mutex);
    flight_departure_t * departure = (flight_departure_t*)arg;
    control_tower_msg msg;
    msg.mtype = DEPARTURE;
    msg.takeoff = departure->takeoff;
    msg.priority = 0;
    if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
        printf("Error sending message through message queue (%s).\n",strerror(errno));
        exit(0);
    }
    pthread_mutex_unlock(&flight_departure_mutex);
    return 0;
}