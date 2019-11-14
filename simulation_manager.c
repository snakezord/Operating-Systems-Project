#include "shared.h"

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

void show_stats(int sig){
	printf("\n\nStatistics:\n\n");
	printf("average_time_take_off: %f\n",sharedMemoryStats->average_time_take_off);
	printf("average_waiting_time_landing: %f\n",sharedMemoryStats ->average_waiting_time_landing);
	printf("Total flights_redirectionated: %d\n",sharedMemoryStats ->flights_redirectionated);
	printf("Total flights_rejected_by_control_tower: %d\n",sharedMemoryStats ->flights_rejected_by_control_tower);
	printf("total_flights_created: %d\n",sharedMemoryStats ->total_flights_created);
    printf("total_flights_landed: %d\n",sharedMemoryStats ->total_flights_landed);
	printf("total_flights_taken_off: %d\n",sharedMemoryStats ->total_flights_taken_off);
}

void init_semaphores(){
    if (sem_init(&(sharedMemoryStats->sem_stats), 1, 1) == -1) {
    	perror("sem_init(): Failed to initialize stats semaphore");
    	exit(-1);
    }

    if (sem_init(&sem_log, 1, 1) == -1) {
    	perror("sem_init(): Failed to initialize log semaphore");
    	exit(-1);
    }
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

}

void create_message_queue(){
    // Message Queue
    msqid = msgget(IPC_PRIVATE, IPC_CREAT|0777);
	if (msqid < -1) {
		logger("msgget(): Failed to create MQ");
		exit(-1);
	}
}

void terminate(int sig){
    logger("Program ended!\n");

    TERMINATE = 1;
    
    //Shared memory detach
	shmdt(&sharedMemoryStats);

    //Remove shared memory
	shmctl(shmidStats, IPC_RMID, NULL);
    
    //Remove message queue
	msgctl(msqid, IPC_RMID, NULL);

    //Destroy stats semaphore
	sem_destroy(&sharedMemoryStats->sem_stats);

    //Destroy log semaphore
	sem_destroy(&sem_log);   

    //Waits for processes to exit
	while(wait(NULL) > 0);
}

void init(){
    
    //criar memoria partilhada
    create_shared_memory();

    init_semaphores();
    init_stats();
    
    //criar message queue
    create_message_queue();

    signal(SIGINT, terminate);
    signal(SIGUSR1,show_stats);

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

    //criar threads
    //create_thread_arrivals();
    //create_thread_departures(); 

}


void create_central_process(){
    if(fork() == 0){
        control_tower();
        exit(0);
    }
}


int main(){

    init();
    
    logger("Program started!\n");

    //create central process
    create_central_process();

    return 0;
}

