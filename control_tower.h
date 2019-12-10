//Roman Zhydyk 2016231789
//Diogo Boinas 2016238042
#if !defined(_CONTROL_TOWER_H_)
#define _CONTROL_TOWER_H_
#include "shared.h"

typedef struct runway_t{
    int TYPE;
    int runway;
    int occupied;
}runway_t;


typedef struct arrival_queue_t{
    char name[256];
    int eta, fuel;
    int slot;
    int priority;
    pthread_t thread;
    struct arrival_queue_t * next;
}arrival_queue_t;


typedef struct departure_queue_t{
    char name[256];
    int takeoff;
    int slot;
    pthread_t thread;
    struct departure_queue_t * next;
}departure_queue_t;


void control_tower();
void * control_tower_arrival(void*arg);
void * control_tower_departure(void*arg);


#endif