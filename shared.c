#include "shared.h"


flight_departure_t* flights_departure = NULL;
flight_arrival_t* flights_arrival = NULL;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t flight_departure_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flight_arrival_mutex = PTHREAD_MUTEX_INITIALIZER;
int TERMINATE = 0;

void print_list_departures(){
    flight_departure_t * current = flights_departure;
    if(current == NULL)
        printf("Departures list is empty!\n");
    while (current != NULL) {
        //printf("name: %s\nid: %d\n", current->name, current->id);
        current = current->next;
    }
}

void print_list_arrivals(){
    flight_arrival_t * current = flights_arrival;
    if(current == NULL)
        printf("Arrivals list is empty!\n");
    while (current != NULL) {
        //printf("name: %s\nid: %d\n", current->name, current->id);
        current = current->next;
    }
}

void append_to_list_departures(flight_departure_t *flight_to_add){
    printf("printing lists");
    int id = 1;
    if(flights_departure == NULL){
        flight_to_add->id = id;
        flights_departure = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    flight_departure_t* current = flights_departure;
    while (current->next != NULL) {
        current = current->next;
        id++;
    }
    flight_to_add->id = id + 1;
    current->next = flight_to_add;
    current->next->next = NULL;
}

void append_to_list_arrivals(flight_arrival_t *flight_to_add){
    printf("printing lists");
    int id = 1;
    if(flights_arrival == NULL){
        flight_to_add->id = id;
        flights_arrival = flight_to_add;
        flight_to_add->next=NULL;
        return;
    }
    flight_arrival_t* current = flights_arrival;
    while (current->next != NULL) {
        current = current->next;
        id++;
    }
    flight_to_add->id = id + 1;
    current->next = flight_to_add;
    current->next->next = NULL;
}

flight_arrival_t * popFirstArrival(flight_arrival_t ** flights_arrival) 
{
    flight_arrival_t * first = *flights_arrival;
    flight_arrival_t * next_node = (*flights_arrival)->next;

    if (*flights_arrival == NULL) {
        return NULL;
    }

    //next_node = (*flights_arrival)->next;
    //first = *flights_arrival;
    free(*flights_arrival);
    *flights_arrival = next_node;
    return first;   
}


flight_departure_t * popFirstDeparture(flight_departure_t ** flights_departure) 
{
    flight_departure_t * first = NULL;
    flight_departure_t * next_node = NULL;

    if (*flights_departure == NULL) {
        return NULL;
    }
    next_node = (*flights_departure)->next;
    first = *flights_departure;
    free(*flights_departure);
    *flights_departure = next_node;
    return first; 
}

flight_arrival_t * popArrivalInit(flight_arrival_t ** flights_arrival, int current_time) 
{
    flight_arrival_t * pop_node = NULL;
    flight_arrival_t * next_node = NULL;

    if (*flights_arrival == NULL) {
        return NULL;
    }
    while(*flights_arrival!=NULL){
        if(time_difference((*flights_arrival)->received_time,current_time)==(*flights_arrival)->init){
            next_node = (*flights_arrival)->next;
            pop_node = *flights_arrival;
            free(*flights_arrival);
            *flights_arrival = next_node;
            return pop_node;   
        }
        next_node = (*flights_arrival)->next;
        *flights_arrival = next_node;
    }
    return NULL;
}


flight_departure_t * popDepartureInit(flight_departure_t ** flights_departure, int current_time) 
{
    flight_departure_t * pop_node = NULL;
    flight_departure_t * next_node = NULL;

    if (*flights_departure == NULL) {
        return NULL;
    }
    while(*flights_departure!=NULL){
        if(time_to_millis(time_difference((*flights_departure)->received_time,current_time))==(*flights_departure)->init){
            next_node = (*flights_departure)->next;
            pop_node = *flights_departure;
            free(*flights_departure);
            *flights_departure = next_node;
            return pop_node;   
        }
        next_node = (*flights_departure)->next;
        *flights_departure = next_node;
    }
    return NULL;
}


int count_total_arrivals(){
    int total = 0;
    flight_arrival_t * current = flights_arrival;
    while (current != NULL) {
        total += 1;
        current = current->next;
    }
    //printf("total arr = %d\n", (total+1));
    return total;
}

int count_total_departures(){
    int total = 0;
    flight_departure_t * current = flights_departure;
    while (current != NULL) {
        total += 1;
        current = current->next;
    }
    //printf("total dep = %d\n", (total+1));
    return total;
}
int get_current_time(){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    return start.tv_sec;
}

int time_difference(int start, int end){
    int delta_us = (end - start) * 1000000 + (end - start) / 1000;
    return delta_us;
}


//nano to milliseconds
int time_to_millis(int time){
    return time/1000;
}


void* thread_time_func(void*arg){
    current_time->milliseconds = 0;
    if(current_time->milliseconds < get_current_time()){
        current_time->milliseconds = get_current_time();
    }
    return 0;
}