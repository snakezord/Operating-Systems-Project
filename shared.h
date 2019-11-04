#if !defined(_SHARED_H_)
#define _SHARED_H_

#define MAX_TEXT 1024

typedef struct{
    int time_unit;
    int takeoff_duration, takeoff_interval;
    int landing_duration, landing_interval;
    int holding_min_duration, holding_max_durarion;
    int max_departures_on_system;
    int max_arrivals_on_system;
}settings_t;

#endif