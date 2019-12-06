#if !defined(_FLIGHT_H_)
#define _FLIGHT_H_
#define MAX_TEXT 1024
#include "shared.h"


typedef struct flight_departure_t{
    char name[MAX_TEXT];
    int received_time;
    int init, takeoff, id;
    pthread_t thread;
    struct flight_departure_t *next;
}flight_departure_t;

typedef struct flight_arrival_t{
    char name[MAX_TEXT];
    int received_time;
    int init, eta, fuel, id;
    pthread_t thread;
    struct flight_arrival_t *next;
}flight_arrival_t;

typedef struct control_tower_msg{
    long mtype;
    int takeoff;
    int eta,fuel;
    int priority;
}control_tower_msg;

/*typedef struct flight{
    flight_arrival_t flight_a;
    flight_departure_t flight_d;
    struct flight *next;
}flight_t;*/


void * flight_arrival(void*arg);
void * flight_departure(void*arg);

#endif