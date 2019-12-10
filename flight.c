//Roman Zhydyk 2016231789
//Diogo Boinas 2016238042
#include "shared.h"
void * flight_arrival(void*arg){
    sem_wait(sem_stats);
    sharedMemoryStats->total_flights_created++;
    sem_post(sem_stats);

    flight_arrival_t * arrival = (flight_arrival_t*)arg;
    //deve arrancar apenas no instante init correspondente
    msleep(arrival->init);
    char * name = arrival->name;
    //Cada thread deverá escrever no log o tempo em que foi iniciada 
    log_str("Arrival Thread created: %s",name);
    //Os voos de chegada devem notificar a Torre de Controlo através da fila de
    //mensagens o seu tempo até à pista (ETA) e o seu combustível.
    control_tower_msg_t msg;
    msg.mtype = ARRIVAL;
    msg.eta = arrival->eta;
    msg.fuel = arrival->fuel;
    msg.priority = 0;
    strcpy(msg.name, name);
    printf("Arrival flight: %s Preparing MSQ: mtype =  %ld eta =  %d  fuel = %d priority = %d\n",arrival->name, msg.mtype, msg.eta, msg.fuel, msg.priority);
    //Caso um voo de chegada apenas tenha combustível para voar até ao instante ‘4 +
    //ETA + duração da aterragem’, a mensagem a enviar à Torre de Controlo através
    //da fila de mensagens deve ser prioritária e lida antes das outras.
    //A thread deverá escrever no log o instante em que enviou a mensagem de emergência
    
    if(arrival->fuel <= (4+arrival->eta+settings.landing_duration)){
        printf("Arrival Flight: %s MAYDAY MAYDAY! Need to land fast!\n", arrival->name);
        msg.mtype = PRIORITY;
        msg.priority = 1;
    }
    printf("Arrival Flight: %s sening MSQ to CT\n", arrival->name);

    if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
        logger("Error sending message through message queue flight departure");
        exit(0);
    }

    //Waiting for sharedmemory slot
    printf("Arrival Flight: %s sending request to CT and waiting for shared memory slot\n", arrival->name);
    if(msgrcv(msqid,&msg,sizeof(msg), ARRIVAL,0) < 0){
        logger("Error receiving messagem from message queue flight arrival.\n");
        exit(0); 
    }
    
    //Received shared memory slot
    int slot = msg.slot;

    //As threads correspondentes às chegadas recebem as instruções da Torre de
    //Controlo através da memória partilhada - aterragem, holding, desvio para outro aeroporto.
    int held = 0;
    printf("Arrival flight: %s slot %d received, locked - holding_arrival_mutex\n", arrival->name, slot);
    pthread_mutex_lock(&holding_arrival_mutex);
    while(sharedMemorySlots->slots[slot].holding != 0){
        printf("Arrival Flight: %s On holding, waiting for signal from CT\n", arrival->name);
        msleep(1);
        held = 1;
        sem_wait(sem_stats);
        sharedMemoryStats->average_waiting_time_landing++;
        sem_post(sem_stats);

        pthread_cond_wait(&holding_arrival_condition, &holding_arrival_mutex);
    }
    pthread_mutex_unlock(&holding_arrival_mutex);
    printf("Arrival Flight: %s Signal received from CT - Finished holding\n", arrival->name);

    if(held){
        sem_wait(sem_stats);
        sharedMemoryStats->holding_maneuvers_landing++;
        sem_post(sem_stats);
    }
    printf("Arrival Flight: %s Thread unlocked - holding_arrival_mutex, checking if can land or will detour\n", arrival->name);

    if(sharedMemorySlots->slots[slot].landing == 1){
        //Os voos de chegada demoram L unidades de tempo a aterrar
        printf("Arrival Flight: %s landing...\n", arrival->name);
        msleep(settings.landing_duration);
        printf("Arrival Flight: %s landed, exiting thread\n", arrival->name);
        int runway = sharedMemorySlots->slots[slot].runway; 
        RUNWAYS[runway].occupied = 0; //free runway
        sharedMemorySlots->slots[slot].occupied = 0;//free slot
        msleep(settings.landing_interval);

        sem_wait(sem_stats);
        sharedMemoryStats->total_flights_landed++;
        sem_post(sem_stats);
        pthread_exit(NULL);
    }
    if(sharedMemorySlots->slots[slot].detour == 1){
        printf("Arrival Flight: %s Detouring, exiting thread\n", arrival->name);
        sharedMemorySlots->slots[slot].occupied = 0; //free slot       

        sem_wait(sem_stats);
        sharedMemoryStats->flights_redirectionated++;
        sem_post(sem_stats);
        pthread_exit(NULL);
    }
    printf("Arrival Flight: %s Thread Completed Wrongly, not landed, neither detoured, exiting thread\n", arrival->name);
    pthread_exit(NULL);
}

void * flight_departure(void*arg){
    sem_wait(sem_stats);
    sharedMemoryStats->total_flights_created++;
    sem_post(sem_stats);

    flight_departure_t * departure = (flight_departure_t*)arg;
    //deve arrancar apenas no instante init correspondente
    msleep(departure->init);
    char * name = departure->name;
    log_str("Departure Thread created: %s",name);
    //Os voos de partida, ao serem criados, devem enviar uma mensagem à Torre de
    //Controlo, através da MSQ a dizer qual é o seu instante de partida desejado.
    control_tower_msg_t msg;
    msg.mtype = DEPARTURE;
    msg.takeoff = departure->takeoff;
    msg.priority = 0;
    strcpy(msg.name, name);

    printf("Departure flight: %s Preparing MSQ: mtype =  %ld takeoff = %d priority = %d\n",departure->name, msg.mtype, msg.takeoff, msg.priority);
    
    printf("Departure Flight: %s sening MSQ to CT\n", departure->name);
    if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
        printf("Error sending message through message queue (%s).\n",strerror(errno));
        exit(0);
    }
    printf("Departure Flight: %s waiting MSQ response from CT\n", departure->name);
    //Waiting for sharedmemory slot
    if(msgrcv(msqid,&msg,sizeof(msg), DEPARTURE,0) < 0){
        logger("Error receiving messagem from message queue 3.\n");
        exit(0);
    }
    //Received slot from CT
    int slot = msg.slot;
    //As threads correspondentes às partidas recebem a indicação de descolagem pelo
    //seu slot em memória partilhada.
    
    printf("Departure flight: %s slot %d received, locked - holding_departure_mutex, verifying if hold\n", departure->name, slot);
    pthread_mutex_lock(&holding_departure_mutex);
    while(sharedMemorySlots->slots[slot].holding != 0){
        msleep(1);
        sem_wait(sem_stats);
        sharedMemoryStats->average_time_take_off++;
        sem_post(sem_stats);
        //printf("Departure Flight: %s On holding\n", departure->name);
        pthread_cond_wait(&holding_departure_condition, &holding_departure_mutex);
        printf("Departure Flight: %s Finished holding\n", departure->name);
    }
    pthread_mutex_unlock(&holding_departure_mutex);
    printf("Departure Flight: %s checking if can depart\n", departure->name);
    if(sharedMemorySlots->slots[slot].depart == 1){
        printf("Departure Flight: %s completed departure, exiting thread\n", departure->name);
        int runway = sharedMemorySlots->slots[slot].runway;
        RUNWAYS[runway].occupied = 0; //free runway
        sharedMemorySlots->slots[slot].occupied = 0;//free slot

        sem_wait(sem_stats);
        sharedMemoryStats->total_flights_taken_off++;
        sem_post(sem_stats);
        pthread_exit(NULL);
    }
    printf("Departure Flight: %s Thread Completed Wrongly, not departed, exiting thread\n", departure->name); 
    pthread_exit(NULL);
}