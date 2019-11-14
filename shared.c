#include "shared.h"


flight_departure_t* flights_departure = NULL;
flight_arrival_t* flights_arrival = NULL;
int TERMINATE = 0;

void append_to_list_departures(flight_departure_t * f, flight_departure_t *flight_to_add){
    if(f== NULL){
        f->next=flight_to_add;
        flight_to_add->next = NULL;
    }
    while(f!=NULL){
        f = f->next;
    }
    f->next=f;
    f->next = NULL;
}

void append_to_list_arrivals(flight_arrival_t * f, flight_arrival_t *flight_to_add){
    if(f== NULL){
        f->next=flight_to_add;
        flight_to_add->next = NULL;
    }
    while(f!=NULL){
        f = f->next;
    }
    f->next=f;
    f->next = NULL;
}

int count_total_arrivals(flight_arrival_t* list){
    int i=0;
    if(list==NULL){
        return i;
    }
    while(list!=NULL){
        i++;
        list = list->next;
    }
    return i;
}


int count_total_departures(flight_departure_t* list){
    int i=0;
    if(list==NULL){
        return i;
    }
    while(list!=NULL){
        i++;
        list = list->next;
    }
    return i;
}

flight_arrival_t * popFirstArrival() 
{
    if (flights_arrival == NULL) 
        return NULL; 
    
    // Move the head pointer to the next node 
    flight_arrival_t * to_remove = flights_arrival; 
    flights_arrival = flights_arrival->next; 
    flight_arrival_t * first = to_remove;
    free(to_remove);
    return first; 
}

flight_departure_t * popFirstDeparture() 
{
    if (flights_departure == NULL) 
        return NULL; 

    // Move the head pointer to the next node 
    flight_departure_t * to_remove = flights_departure; 
    flights_departure = flights_departure->next; 
    flight_departure_t * first = to_remove;
    free(to_remove);
    return first; 
}