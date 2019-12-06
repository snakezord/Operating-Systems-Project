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
#define DEPARTURE 1
#define ARRIVAL 2

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

extern pthread_cond_t cond;
extern pthread_mutex_t flight_departure_mutex;
extern pthread_mutex_t flight_arrival_mutex;
//flag for program termination
extern int TERMINATE;
//system settingss
settings_t settings;
//Linked Lists
extern flight_departure_t* flights_departure;
extern flight_arrival_t* flights_arrival;
time_thread_t * current_time;
//Shared mem
int shmidStats;
statistic_t * sharedMemoryStats;
//Semaphore
sem_t * sem_stats;
sem_t * sem_log;
//Message queue
int msqid;
//pipe
int fd_pipe;
fd_set read_set;

void print_list_departures();
void print_list_arrivals();
void append_to_list_departures(flight_departure_t *flight_to_add);
void append_to_list_arrivals(flight_arrival_t *flight_to_add);
int count_total_arrivals();
int count_total_departures();
flight_arrival_t * popFirstArrival(flight_arrival_t ** flights_arrival);
flight_departure_t * popFirstDeparture(flight_departure_t ** flights_departure);
int get_current_time();
int time_difference(int start, int end);
int time_to_millis(int time);
int msleep(long msec);
void* thread_time_func(void*arg);

#endif