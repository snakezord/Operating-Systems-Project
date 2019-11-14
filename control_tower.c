#include "shared.h"

void create_thread_arrivals(){
    while(flights_arrival != NULL){
        flight_arrival_t * arrival = popFirstArrival();
        pthread_create(&(arrival->thread),NULL,flight_arrival,arrival);
    }
}

void create_thread_departures(){
    while(flights_departure != NULL){
        flight_departure_t * departure = popFirstDeparture();
        pthread_create(&(departure->thread),NULL,flight_departure,departure);
    }
}

void control_tower(){
    logger("Central Process Created\n");
    //Named Pipe
    unlink(PIPE_NAME);
    //EEXIST     =     File exists (POSIX.1-2001).
    if((mkfifo(PIPE_NAME, O_CREAT|O_EXCL|0600) < 0) && (errno!=EEXIST)){
        logger("Error creating pipe\n");
        exit(0);
    }
    handle_pipe();
}

void handle_pipe(){
    while(!TERMINATE){
        if((fd_pipe = open(PIPE_NAME, O_RDONLY|O_NONBLOCK))<0){
            logger("Error opening named pipe.\n");
            exit(0);
        }
        while(1){
            char buffer[MAX_TEXT];
            int n = 0;
            char c = 0;
            int error = 0;
            do{
                if(read(fd_pipe,&c,1)<=0){
                    error=1;
                    break;
                }
                if(c != '\n'){
                    buffer[n++] = c;
                }
            }while(c!='\n');
            if(error){
                break;
            }
            logger(buffer);
            parse_request(buffer);
        }
        close(fd_pipe);
    }
    exit(0);
}

void parse_request(char *str){
    char *buffer;
    buffer = strtok(str, " ");
    int VALID_COMMAND = 0;
    if(strcmp(buffer, "ARRIVAL")==0){
        flight_arrival_t *flight = malloc(sizeof(flight_arrival_t));
        strcpy(flight->name, strtok(NULL," "));
        strtok(NULL," ");
        flight->init= atoi(strtok(NULL," "));
        strtok(NULL, " ");
        flight->takeoff = atoi(strtok(NULL," "));
        if(((flight->takeoff)-(flight->init))>=0){
            if((count_total_arrivals(flights_arrival) + 1) <= settings.max_arrivals_on_system){
                append_to_list_arrivals(flights_arrival,flight);
                logger("ARRIVAL received\n");
            }else{
                logger("You have exceed the total number of arrivals for this airport");
            }
        }else{
            logger("Takeoff time is less than init time\n");
        }
        VALID_COMMAND = 1;
    }else if(strcmp(buffer, "DEPARTURE")==0){
        flight_departure_t *flight = malloc(sizeof(flight_departure_t));
        strcpy(flight->name, strtok(NULL," "));
        strtok(NULL," ");
        flight->init = atoi(strtok(NULL," "));
        strtok(NULL," ");
        flight->eta = atoi(strtok(NULL," "));
        strtok(NULL," ");
        flight->fuel = atoi(strtok(NULL," "));
        if(((flight->fuel)-(flight->eta))>=0){
            if((count_total_departures(flights_departure) + 1) <= settings.max_departures_on_system){
                append_to_list_departures(flights_departure,flight);
                logger("DEPARTURE received\n");
            }else{
                logger("You have exceed the total number of departures for this airport");
            }
        }else{
            logger("Plane doesn't have enough fuel to get to the airport");
        }
         VALID_COMMAND = 1;
    }if(!VALID_COMMAND){
        logger("Invalid command!\n");
    }
}