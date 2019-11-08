#include "shared.h"
#include "flight.h"

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