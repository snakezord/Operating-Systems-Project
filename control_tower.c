#include "shared.h"
#include "flight.h"
 
 int fd_pipe=-1;

int control_tower(){
    //Named Pipe
    unlink(PIPE_NAME);
    if(mkfifo(PIPE_NAME, O_CREAT|0666)<0){
        perror("Error creating pipe\n");
        exit(1);
    }

    /*
    //Create flights threads
    flight_departure_t *tmp = flights_departure;
    for(int i=0;i<settings.max_departures_on_system;i++){
        flight_departure_t *param = tmp;
        pthread_create(&(tmp->thread),NULL,flight_arrival,param);
        tmp = tmp->next;
    }

    flight_arrival_t *tmp = flights_arrival;
    for(int i=0;i<settings.max_arrivals_on_system;i++){
        flight_arrival_t *param = tmp;
        pthread_create(&(tmp->thread),NULL,flight_departure,param);
        tmp=tmp->next;
    }*/
}

void handle_pipe(){
    while(1){
        if((fd_pipe = open(PIPE_NAME, O_RDONLY)<0)){
            perror("Error opening named pipe.\n");
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
            parse_request(buffer);
        }
        close(fd_pipe);
    }
}

void parse_request(char *str){
    char *buffer;
    buffer = strtok(str, " ");
    if(strcmp(buffer, "ARRIVAL")==0){
        flight_arrival_t *flight = malloc(sizeof(flight_arrival_t));
        strcpy(flight->name, strtok(NULL," "));
        strtok(NULL," "); 
        flight->init= atoi(strtok(NULL," "));
        strtok(NULL, " ");
        flight->takeoff = atoi(strtok(NULL," "));
        if(((flight->takeoff)-(flight->init))<=0){
            perror("Takeoff time is equal to init time\n");
            return;
        }
        if(count_total_arrivals(flights_arrival) >= settings.max_arrivals_on_system){
            perror("You have exceed the total number of arrivals for this airport");
            return;
        }
        //criação da thread
        pthread_create(&(flight->thread),NULL,flight_arrival,flight);
        //adicionar a linkedlist
        append_to_list_arrivals(flights_arrival,flight);
    }else if(strcmp(buffer, "DEPARTURE")==0){
        flight_departure_t *flight = malloc(sizeof(flight_departure_t));
        strcpy(flight->name, strtok(NULL," "));
        strtok(NULL," ");
        flight->init = atoi(strtok(NULL," "));
        strtok(NULL," ");
        flight->eta = atoi(strtok(NULL," "));
        strtok(NULL," ");
        flight->fuel = atoi(strtok(NULL," "));
        if(((flight->fuel)-(flight->eta))<=0){
            perror("Plane doesn't have enough fuel to get to the airport");
            return;
        }
        if(count_total_departures(flights_departure) >= settings.max_departures_on_system){
            perror("You have exceed the total number of departures for this airport");
            return;
        }
        //criação da thread
        pthread_create(&(flight->thread),NULL,flight_departure,flight);
        //adicionar a linked list
        append_to_list_departures(flights_departure,flight);
    }
}

