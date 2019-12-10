//Roman Zhydyk 2016231789
//Diogo Boinas 2016238042
#include "shared.h"

void control_tower(){
    int slot_pos;
    slots_struct_t slot;
    control_tower_msg_t msg;
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, show_stats);
    logger("Control Tower - Central Process Created\n");
    while(!TERMINATE){
        printf("Control Tower: Waiting for MSQ message from any flight, receiving first element from MSQ\n");
        if(msgrcv(msqid,&msg,sizeof(msg),0,0) < 0){
            logger("Error receiving messagem from message queue 2.\n");
            exit(0); 
        }
        
        printf("Control Tower: Received message from MSQ\n");
        if(msg.mtype == ARRIVAL || msg.mtype == PRIORITY){
            int fuel = msg.fuel;
            int eta =  msg.eta;   
            char name[MAX_TEXT];
            strcpy(name, msg.name);
            int priority = msg.priority;
            printf("Control Tower: Arrival %s received - fuel = %d eta = %d priority = %d\n", name, fuel, eta, priority);
            //A Torre de Controlo responderá pela mesma MSQ com o
            //número do slot em memória partilhada atribuído a esse voo específico. Será
            //através desse slot que as threads receberão a indicação de descolagem/aterragem,
            //bem como das informações de necessidade de uma manobra de holding ou de
            //desvio para outro aeroporto

            printf("Control Tower: Generating slot for Arrival %s\n", name);
            sem_wait(sem_slots);
            slot_pos = get_empty_slot(); //returns first empty slot and occupy it
            sem_post(sem_slots);
            printf("Control Tower: Slot %d generated for Arrival %s\n", slot_pos, name);
            slot.type = ARRIVAL;

            msg.slot = slot_pos;
            msg.mtype = ARRIVAL;
            printf("Control Tower: Sending slot to Arrival %s | msg.slot = %d msg.mtype = %ld \n", name, msg.slot, msg.mtype);
            if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
                printf("Error sending message through message queue (%s).\n",strerror(errno));
                exit(0);
            }
            
            //Caso a Torre de Controlo verifique que é necessário adiar a aterragem,
            //o voo será ordenado a realizar uma manobra de holding que irá alterar o ETA
            printf("Control Tower: Preparing Arrival flight %s to Arrivals QUEUE\n", name);
            arrival_queue_t * arrival = (arrival_queue_t*)malloc(sizeof(arrival_queue_t));
            strcpy(arrival->name, name);
            arrival->eta = eta;
            arrival->fuel = fuel;
            arrival->slot = slot_pos;
            arrival->priority = priority;
            printf("Control Tower: Arrival %s prepared to Arrivals QUEUE - fuel = %d eta = %d slot = %d priority = %d\n", arrival->name, arrival->fuel, arrival->eta, arrival->slot,arrival->priority);
            //Deve também colocar o voo na fila de espera para
            //partidas, de acordo com o instante desejado de partida.
            append_to_queue_arrivals(arrival);
            printf("Control Tower: Arrival flight %s added to Arrivals QUEUE\n", name);
            ArrivalMergeSort(&arrival_queue);
            printf("Control Tower: Arrivals QUEUE SORTED by eta, \n");
            if(pthread_create(&arrival->thread,NULL,control_tower_arrival,(void*)arrival)!=0){
                logger("Error creating control tower Arrival routine");
                exit(0);
            }
        }
        if(msg.mtype == DEPARTURE){
            char name[MAX_TEXT];
            int takeoff = msg.takeoff;
            strcpy(name, msg.name);
            printf("Control Tower: Departure %s received - takeoff = %d\n", name, takeoff);
            //A torre deve guardar a informação recebida e responder pela
            //mesma MSQ com o número do slot no espaço de memória partilhada que é
            //atribuído a esse voo específico.
            printf("Control Tower: Generating slot for Departure %s\n", name);
            sem_wait(sem_slots);
            slot_pos = get_empty_slot(); //returns first empty slot and occupy it
            sem_post(sem_slots);
            printf("Control Tower: Slot %d generated for Departure %s\n", slot_pos, name);    
            slot.type = DEPARTURE;
            msg.slot = slot_pos;
            msg.mtype = DEPARTURE;
            printf("Control Tower: Sending slot to Departure %s | msg.slot = %d msg.mtype = %ld \n", name, msg.slot, msg.mtype);
            if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
                printf("Error sending message through message queue (%s).\n",strerror(errno));
                exit(0);
            }
            //Deve também colocar o voo na fila de espera para
            //partidas, de acordo com o instante desejado de partida.
            printf("Control Tower: Preparing Departure flight %s to Departure QUEUE\n", name);
            departure_queue_t * departure = (departure_queue_t*)malloc(sizeof(departure_queue_t));
            strcpy(departure->name,name);
            departure->takeoff = takeoff;
            departure->slot = slot_pos;
            append_to_queue_departures(departure);
            printf("Control Tower: Departure flight %s added to Departure QUEUE\n", name);
            DepartureMergeSort(&departure_queue);
            printf("Control Tower: Departure QUEUE SORTED by takeoff\n");
            if(pthread_create(&departure->thread,NULL,control_tower_departure,(void*)departure)){
                logger("Error creating control tower routine");
            }
        }
    }
    //Finish Arrival and Departure threads
    arrival_queue_t* current_arrival = arrival_queue;
    while (current_arrival->next != NULL) {
        pthread_join(current_arrival->thread, NULL);
        current_arrival = current_arrival->next;
    }
    departure_queue_t* current_departure = departure_queue;
    while (current_departure->next != NULL) {
        pthread_join(current_departure->thread, NULL);
        current_departure = current_departure->next;
    }
}

//Usar o mesmo mutex nas threads?..Ct e flight

void * control_tower_arrival(void*arg){

    arrival_queue_t * arrival = (arrival_queue_t*)arg;
    printf("Control Tower - Arrival flight %s MANAGEMENT thread created, fuel = %d eta = %d priority = %d\n", arrival->name, arrival->fuel, arrival->eta, arrival->priority);
    //A Torre de Controlo deverá marcar voos prioritários como urgentes e evitar que 
    //eles tenham de efetuar manobras de holding.
    printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Flight APPROXIMATING. fuel = %d eta = %d . Decrementing eta and fuel every ms\n", arrival->name, arrival->fuel, arrival->eta);
    while(arrival->eta!=0){
        //printf("Control Tower - Arrival flight %s MANAGEMENT Thread: eta = %d, fuel = %d\n", arrival->name, arrival->eta, arrival->fuel);
        arrival->eta--;
        arrival->fuel--;
        if(arrival->fuel == 0){
            printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Fuel reached 0 , sending to detour threw sharedMemorySlots!\n", arrival->name);
            sharedMemorySlots->slots[arrival->slot].detour = 1;
            printf("Control Tower - Arrival flight %s MANAGEMENT Thread: TERMINATING\n", arrival->name);
            //sem_post(sem_slots);
            pthread_exit(NULL);
        }
        msleep(1);
    }
    printf("Control Tower - Arrival flight %s MANAGEMENT Thread: FLIGHT ARRIVED, fuel = %d eta = %d . Checking for avaliable runway\n", arrival->name, arrival->fuel, arrival->eta);
    pthread_mutex_lock(&flight_arrival_mutex);
    if(RUNWAYS[L_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
        RUNWAYS[L_01].occupied = 1;
        log_str("Control Tower - Arrival flight MANAGEMENT Thread: Runway L01 assigned to flight %s",arrival->name);
        printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Locking - holding_arrival_mutex, signaling - holding_arrival_condition\n", arrival->name);
        pthread_mutex_lock(&holding_arrival_mutex);
        sharedMemorySlots->slots[arrival->slot].holding = 0;
        pthread_cond_broadcast(&holding_arrival_condition);
        pthread_mutex_unlock(&holding_arrival_mutex);  
        sharedMemorySlots->slots[arrival->slot].landing = 1;
        sharedMemorySlots->slots[arrival->slot].runway = L_01;
        pthread_mutex_unlock(&flight_arrival_mutex);
        printf("Control Tower - Arrival flight %s MANAGEMENT Thread: unlocking holding_arrival_mutex, TERMINATING\n", arrival->name);
        pthread_exit(NULL);
    }else if(RUNWAYS[R_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
        RUNWAYS[R_01].occupied = 1;
        log_str("Control Tower - Arrival flight MANAGEMENT Thread: Runway R_01 assigned to flight %s",arrival->name);
        printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Locking - holding_arrival_mutex, signaling - holding_arrival_condition\n", arrival->name);
        pthread_mutex_lock(&holding_arrival_mutex);
        sharedMemorySlots->slots[arrival->slot].holding = 0;
        pthread_cond_broadcast(&holding_arrival_condition);
        pthread_mutex_unlock(&holding_arrival_mutex);  
        sharedMemorySlots->slots[arrival->slot].landing = 1;
        sharedMemorySlots->slots[arrival->slot].runway = R_01;
        pthread_mutex_lock(&flight_arrival_mutex);
        printf("Control Tower - Arrival flight %s MANAGEMENT Thread: unlocking holding_arrival_mutex, TERMINATING\n", arrival->name);
        pthread_exit(NULL);
    }else{
        log_str("Control Tower - Arrival flight MANAGEMENT Thread: Runways for arrival occupied, holding on reserve, flight %s",arrival->name);
        while(arrival->fuel != 0){
            printf("Control Tower - Arrival flight %s MANAGEMENT Thread: fuel = %d\n", arrival->name, arrival->fuel);
            arrival->fuel--;
            if(RUNWAYS[L_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
                RUNWAYS[L_01].occupied = 1;
                log_str("Control Tower - Arrival flight MANAGEMENT Thread: Runway L01 assigned to flight %s",arrival->name);
                printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Locking - holding_arrival_mutex, signaling - holding_arrival_condition\n", arrival->name);
                pthread_mutex_lock(&holding_arrival_mutex);
                sharedMemorySlots->slots[arrival->slot].holding = 0;
                pthread_cond_broadcast(&holding_arrival_condition);
                pthread_mutex_unlock(&holding_arrival_mutex);   
                sharedMemorySlots->slots[arrival->slot].landing = 1;
                sharedMemorySlots->slots[arrival->slot].runway = L_01;
                pthread_mutex_unlock(&flight_arrival_mutex);
                printf("Control Tower - Arrival flight %s MANAGEMENT Thread: unlocking holding_arrival_mutex, TERMINATING\n", arrival->name);
                pthread_exit(NULL);
            }else if(RUNWAYS[R_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
                RUNWAYS[R_01].occupied = 1;
                log_str("Control Tower - Arrival flight MANAGEMENT Thread: Runway R_01 assigned to flight %s",arrival->name);
                printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Locking - holding_arrival_mutex, signaling - holding_arrival_condition\n", arrival->name);
                pthread_mutex_lock(&holding_arrival_mutex);
                sharedMemorySlots->slots[arrival->slot].holding = 0;
                pthread_cond_broadcast(&holding_arrival_condition);
                pthread_mutex_unlock(&holding_arrival_mutex); 
                sharedMemorySlots->slots[arrival->slot].landing = 1;
                sharedMemorySlots->slots[arrival->slot].runway = R_01;
                pthread_mutex_unlock(&flight_arrival_mutex);
                printf("Control Tower - Arrival flight %s MANAGEMENT Thread: unlocking holding_arrival_mutex, TERMINATING\n", arrival->name);
                pthread_exit(NULL);
            }
            msleep(1);
        }
        log_str("Control Tower - Arrival flight MANAGEMENT Thread: Fuel = 0, DETOURING flight %s",arrival->name);
        printf("Control Tower - Arrival flight %s MANAGEMENT Thread: Locking - holding_arrival_mutex, signaling - holding_arrival_condition\n", arrival->name);
        pthread_mutex_lock(&holding_arrival_mutex);
        sharedMemorySlots->slots[arrival->slot].holding = 0;
        pthread_cond_broadcast(&holding_arrival_condition);
        pthread_mutex_unlock(&holding_arrival_mutex); 
        sharedMemorySlots->slots[arrival->slot].detour = 1;
        pthread_mutex_unlock(&flight_arrival_mutex);
        printf("Control Tower - Arrival flight %s MANAGEMENT Thread: unlocking holding_arrival_mutex, TERMINATING\n", arrival->name);
        pthread_exit(NULL);
    }
}


void * control_tower_departure(void*arg){
    departure_queue_t * departure = (departure_queue_t*)arg;
    printf("Control Tower - Departure flight %s MANAGEMENT thread created, takeoff = %d\n", departure->name, departure->takeoff);
    printf("Control Tower - Departure flight %s MANAGEMENT Thread: Flight PREPARING TO TAKEOFF in %d ms. Decrementing takeoff every ms\n", departure->name, departure->takeoff);
    while(departure->takeoff!=0){
        departure->takeoff--;
        msleep(1);
    }
     printf("Control Tower - Departure flight %s MANAGEMENT Thread: FLIGHT READY TO DEPART, checking for avaliable runway\n", departure->name);
    //Assim que receber a ordem para descolar, a thread deve tirar partido dos 
    //mecanismos de sincronização para aguardar, sem espera ativa, que uma das pistas esteja vazia, 
    //que o aeroporto possa receber descolagens e que nenhum voo esteja a aterrar. 
    //Poderá então começar o processo de descolagem, que demora T unidades de tempo.
    pthread_mutex_lock(&flight_departure_mutex);
    if(RUNWAYS[L_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
        RUNWAYS[L_28].occupied = 1;
        log_str("Control Tower - Departure flight MANAGEMENT Thread: Runway L_28 assigned to flight %s", departure->name);
        printf("Control Tower - Departure flight %s MANAGEMENT Thread: Locking - holding_departure_mutex, signaling - holding_departure_condition\n", departure->name);
        
        pthread_mutex_lock(&holding_departure_mutex);
        sharedMemorySlots->slots[departure->slot].holding = 0;
        pthread_cond_broadcast(&holding_departure_condition);
        pthread_mutex_unlock(&holding_departure_mutex);
        sharedMemorySlots->slots[departure->slot].depart = 1;
        sharedMemorySlots->slots[departure->slot].runway = L_28;
        pthread_mutex_unlock(&flight_departure_mutex);
        printf("Control Tower - Departure flight %s MANAGEMENT Thread: unlocking holding_departure_mutex, TERMINATING\n", departure->name);
        pthread_exit(NULL);
    }else if(RUNWAYS[R_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
        RUNWAYS[R_28].occupied = 1;
        log_str("Control Tower - Departure flight MANAGEMENT Thread: Runway R_28 assigned to flight %s", departure->name);
        printf("Control Tower - Departure flight %s MANAGEMENT Thread: Locking - holding_departure_mutex, signaling - holding_departure_condition\n", departure->name);
        pthread_mutex_lock(&holding_departure_mutex);
        sharedMemorySlots->slots[departure->slot].holding = 0;
        pthread_cond_broadcast(&holding_departure_condition);
        pthread_mutex_unlock(&holding_departure_mutex);

        sharedMemorySlots->slots[departure->slot].depart = 1;
        sharedMemorySlots->slots[departure->slot].runway = R_28;
        pthread_mutex_unlock(&flight_departure_mutex);
        printf("Control Tower - Departure flight %s MANAGEMENT Thread: unlocking holding_departure_mutex, TERMINATING\n", departure->name);
        pthread_exit(NULL);
    }else{
        int occupied = 1;
        
        sharedMemorySlots->slots[departure->slot].holding = 1;
       
        while(occupied){
            if(RUNWAYS[L_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
                occupied  = 0;
                RUNWAYS[L_28].occupied = 1;
                log_str("Control Tower - Departure flight MANAGEMENT Thread: Runway L_28 assigned to flight %s", departure->name);
                printf("Control Tower - Departure flight %s MANAGEMENT Thread: Locking - holding_departure_mutex, signaling - holding_departure_condition\n", departure->name);
                pthread_mutex_lock(&holding_departure_mutex);
                sharedMemorySlots->slots[departure->slot].holding = 0;
                pthread_cond_broadcast(&holding_departure_condition);
                pthread_mutex_unlock(&holding_departure_mutex);
                sharedMemorySlots->slots[departure->slot].depart = 1;
                sharedMemorySlots->slots[departure->slot].runway = L_28;
                pthread_mutex_unlock(&flight_departure_mutex);
                printf("Control Tower - Departure flight %s MANAGEMENT Thread: unlocking holding_departure_mutex, TERMINATING\n", departure->name);
                pthread_exit(NULL);
            }else if(RUNWAYS[R_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
                occupied = 0;
                RUNWAYS[R_28].occupied = 1;
                log_str("Control Tower - Departure flight MANAGEMENT Thread: Runway R_28 assigned to flight %s", departure->name);
                printf("Control Tower - Departure flight %s MANAGEMENT Thread: Locking - holding_departure_mutex, signaling - holding_departure_condition\n", departure->name);
                pthread_mutex_lock(&holding_departure_mutex);
                sharedMemorySlots->slots[departure->slot].holding = 0;
                pthread_cond_broadcast(&holding_departure_condition);
                pthread_mutex_unlock(&holding_departure_mutex);
                sharedMemorySlots->slots[departure->slot].depart = 1;
                sharedMemorySlots->slots[departure->slot].runway = R_28;
                pthread_mutex_unlock(&flight_departure_mutex);
                printf("Control Tower - Departure flight %s MANAGEMENT Thread: unlocking holding_departure_mutex, TERMINATING\n", departure->name);
                pthread_exit(NULL);
            }
            msleep(1);
        }
    }
    pthread_mutex_unlock(&flight_departure_mutex);
    pthread_exit(NULL);
}

