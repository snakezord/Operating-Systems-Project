#if !defined(_SIMULATION_MANAGER_H_)
#define _SIMULATION_MANAGER_H_

#define FILENAME "config.txt"
#include "shared.h"



void print_struct();
int read_config_file();
void init_stats();
void init_semaphores();
void create_shared_memory();
void create_message_queue();
void signals();
void init();

#endif