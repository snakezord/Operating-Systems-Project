#if !defined(_SIMULATION_MANAGER_H_)
#define _SIMULATION_MANAGER_H_

#define FILENAME "config.txt"
#include "shared.h"


void print_struct();
int read_config_file();
void init_stats();
void show_stats(int sig);
void init_semaphores();
void create_shared_memory();
void create_message_queue();
void create_pipe();
void terminate(int sig);
void init();
void create_central_process();
int handle_pipe();
int parse_request(char *str);
void create_thread_arrivals();
void create_thread_departures();


#endif