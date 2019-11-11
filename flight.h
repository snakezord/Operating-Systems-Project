#if !defined(_FLIGHT_H_)
#define _FLIGHT_H_
#define MAX_TEXT 1024
#include "shared.h"


typedef struct flight_arrival_t{
    char name[MAX_TEXT];
    int init, takeoff;
    pthread_t thread;
    struct flight_arrival_t *next;
}flight_arrival_t;

typedef struct flight_departure_t{
    char name[MAX_TEXT];
    int init, eta, fuel;
    pthread_t thread;
    struct flight_departure_t *next;
}flight_departure_t;

/*typedef struct flight{
    flight_arrival_t flight_a;
    flight_departure_t flight_d;
    struct flight *next;
}flight_t;*/


void * flight_arrival(void*arg);
void * flight_departure(void*arg);

#endif