#include "shared.h"
#include "simulation_manager.h"

void print_struct(){
    printf("%d\n", settings.time_unit);
    printf("%d %d\n", settings.takeoff_duration, settings.takeoff_interval);
    printf("%d %d\n", settings.landing_duration, settings.landing_interval);
    printf("%d %d\n", settings.holding_min_duration, settings.holding_max_duration);
    printf("%d\n", settings.max_departures_on_system);
    printf("%d\n", settings.max_arrivals_on_system);
}

int read_config_file(){
    FILE * file;
    file = fopen(FILENAME, "r");
    if(file==NULL){
        return -1;
    }
    char line[MAX_TEXT];
    if(fscanf(file,"%d", &settings.time_unit)!=1){
        fclose(file);
        return -1;
    }

    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d, %d", &settings.takeoff_duration, &settings.takeoff_interval)!=2){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d, %d",&settings.landing_duration, &settings.landing_interval)!=2){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d, %d", &settings.holding_min_duration, &settings.holding_max_duration)!=2){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d", &settings.max_departures_on_system)!=1){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d",&settings.max_arrivals_on_system)!=1){
        fclose(file);
        return -1;
    }
    fclose(file);
    return 1;
}


void init_stats(){
    sharedMemoryStats->average_time_take_off = 0;
    sharedMemoryStats->average_waiting_time_landing = 0;
    sharedMemoryStats->flights_redirectionated = 0;
    sharedMemoryStats->flights_rejected_by_control_tower = 0;
    sharedMemoryStats->holding_maneuvers_landing = 0 ;
    sharedMemoryStats->holding_maneuvers_may_day = 0;
    sharedMemoryStats->total_flights_created  = 0;
    sharedMemoryStats->total_flights_landed = 0;
    sharedMemoryStats->total_flights_taken_off = 0;
}



void create_shared_memory(){
    //Initiating Shared memory for Statistics
	shmidStats = shmget(IPC_PRIVATE, sizeof(statistic_t), IPC_CREAT|0666);
    if (shmidStats == -1) {
    	logger("shmget(): Failed to create shared memory for statistics");
    	exit(-1);
    }
    //Attatching shared memory for Statistics
    sharedMemoryStats = shmat(shmidStats, NULL, 0);
    if (*((int *) sharedMemoryStats) == -1) {
    	logger("shmat(): Failed to attach memory for statistics");
    	exit(-1);
    }
    init_stats();

    //Initiating Shared memory for arrivals
	shmidArrivals = shmget(IPC_PRIVATE, sizeof(flight_arrival_t), IPC_CREAT|0666);
    if (shmidArrivals == -1) {
    	logger("shmget(): Failed to create shared memory for arrivals");
    	exit(-1);
    }
    //Attatching shared memory for arrivals
    sharedMemoryArrivals = shmat(shmidArrivals, NULL, 0);
    if (*((int *) sharedMemoryArrivals) == -1) {
    	logger("shmat(): Failed to attach memory for arrivals");
    	exit(-1);
    }

    //Initiating Shared memory for departures
	shmidDepartures = shmget(IPC_PRIVATE, sizeof(flight_departure_t), IPC_CREAT|0666);
    if (shmidDepartures == -1) {
    	logger("shmget(): Failed to create shared memory for departures");
    	exit(-1);
    }
    //Attatching shared memory for departures
    sharedMemoryDepartures = shmat(shmidDepartures, NULL, 0);
    if (*((int *) sharedMemoryDepartures) == -1) {
    	logger("shmat(): Failed to attach memory for departures");
    	exit(-1);
    }
}



void create_message_queue(){
    // Message Queue
    msqid = msgget(IPC_PRIVATE, IPC_CREAT|0777);
	if (msqid < -1) {
		logger("msgget(): Failed to create MQ");
		exit(-1);
	}
}


void create_central_process(){
    if(fork() == 0){
        printf("chegou aqui\n");
        control_tower();
        exit(0);
    }
}


int main(){
    //inicializar logs
    init_logs();
    //ler ficheiro config.txt
    if(read_config_file()){
        logger("Config read successfully\n");
    }  
    else{
        logger("Error in reading config\n");
        exit(0);
    }
    //criar memoria partilhada
    create_shared_memory();
    //criar message queue
    create_message_queue();
    //criar threads
    create_thread_arrivals();
    create_thread_departures();    
    //criar central process
    create_central_process();
    return 0;
}

