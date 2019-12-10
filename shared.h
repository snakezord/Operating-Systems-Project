//Roman Zhydyk 2016231789
//Diogo Boinas 2016238042
#if !defined(_SHARED_H_)
#define _SHARED_H_

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>
#include <time.h>
#include "simulation_manager.h"
#include "control_tower.h"
#include "flight.h"
#include "logs.h"


#define PIPE_NAME "my_pipe"
#define FILESIZE 4194304 // 4 MB
#define DEBUG
#define MAX 200
#define STRINGSIZE 256
#define MAX_TEXT 1024
#define DEPARTURE 2
#define ARRIVAL 3
#define PRIORITY 1
#define L_01 0
#define R_01 1
#define L_28 2
#define R_28 3

typedef struct{
    int time_unit;
    int takeoff_duration, takeoff_interval;
    int landing_duration, landing_interval;
    int holding_min_duration, holding_max_duration;
    int max_departures_on_system;
    int max_arrivals_on_system;
}settings_t;

typedef struct{
    int total_flights_created;
    int total_flights_landed;
    double average_waiting_time_landing;
    int total_flights_taken_off;
    double average_time_take_off;
    double holding_maneuvers_landing;
    double holding_maneuvers_may_day;
    int flights_redirectionated;
    int flights_rejected_by_control_tower;
}statistic_t;

typedef struct{
    int milliseconds;
    pthread_t time_thread;
}time_thread_t;


//thead sync
extern pthread_mutex_t holding_arrival_mutex;
extern pthread_cond_t holding_arrival_condition;

extern pthread_mutex_t holding_departure_mutex;
extern pthread_cond_t holding_departure_condition;

extern pthread_mutex_t flight_departure_mutex;
extern pthread_mutex_t flight_arrival_mutex;
//CT thread mutex
extern pthread_mutex_t CT_flight_departure_MANAGEMENT_mutex;
extern pthread_mutex_t CT_flight_arrival_MANAGEMENT_mutex;
//flag for program termination
extern int TERMINATE;
//system settingss
settings_t settings;
//Linked Lists
extern flight_departure_t* flights_departure;
extern flight_arrival_t* flights_arrival;
extern flight_departure_t* flights_departure_copy;
extern flight_arrival_t* flights_arrival_copy;
//queues
extern arrival_queue_t* arrival_queue;
extern departure_queue_t* departure_queue;
//Shared mem
int shmidStats;
statistic_t * sharedMemoryStats;
int shmidSlots;
shm_slots_struct_t * sharedMemorySlots;
int shmidRunways;
runway_t * RUNWAYS;
//Semaphore
sem_t * sem_stats;
sem_t * sem_log;
sem_t * sem_slots;
sem_t * sem_runways;
//Message queue
int msqid;

//pipe
int fd_pipe;
fd_set read_set;

void print_list_departures();
void print_list_arrivals();
void append_to_list_departures(flight_departure_t *flight_to_add);
void append_to_list_departures_copy(flight_departure_t *flight_to_add);
void append_to_queue_departures(departure_queue_t *flight_to_add);
void append_to_list_arrivals(flight_arrival_t *flight_to_add);
void append_to_list_arrivals_copy(flight_arrival_t *flight_to_add);
void append_to_queue_arrivals(arrival_queue_t *flight_to_add);
int count_total_arrivals();
int count_total_departures();
int count_total_arrivals_queue();
int count_total_departures_queue();
flight_arrival_t * popFirstArrival(flight_arrival_t ** flights_arrival);
flight_departure_t * popFirstDeparture(flight_departure_t ** flights_departure);
void popFirstArrivalQueue(arrival_queue_t ** flights_arrival);
void popFirstDepartureQueue(departure_queue_t ** flights_departure);
int msleep(long msec);
int get_empty_slot();
int cmp( const void *left, const void *right );
void DepartureMergeSort(departure_queue_t** headRef);
departure_queue_t* DepartureSortedMerge(departure_queue_t* a, departure_queue_t* b);
void DepartureFrontBackSplit(departure_queue_t* source, departure_queue_t** frontRef, departure_queue_t** backRef);
void ArrivalMergeSort(arrival_queue_t** headRef);
arrival_queue_t* ArrivalSortedMerge(arrival_queue_t* a, arrival_queue_t* b);
void ArrivalFrontBackSplit(arrival_queue_t* source, arrival_queue_t** frontRef, arrival_queue_t** backRef);

#endif