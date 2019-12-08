#include "shared.h"

void control_tower(){
    int slot_pos;
    slots_struct slot;
    control_tower_msg_t msg;
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, show_stats);
    logger("Control Tower - Central Process Created\n");
    while(!TERMINATE){
        if(msgrcv(msqid,&msg,sizeof(msg),0,0) < 0){
            logger("Error receiving messagem from message queue 2.\n");
            exit(0); 
        }
        printf("m_type %ld\n", msg.mtype);
        if(msg.mtype == ARRIVAL || msg.mtype == PRIORITY){
            int fuel = msg.fuel;
            int eta =  msg.eta;   
            char name[MAX_TEXT];
            strcpy(name, msg.name);
            //A Torre de Controlo responderá pela mesma MSQ com o
            //número do slot em memória partilhada atribuído a esse voo específico. Será
            //através desse slot que as threads receberão a indicação de descolagem/aterragem,
            //bem como das informações de necessidade de uma manobra de holding ou de
            //desvio para outro aeroporto

            //Caso qualquer voo receba como resposta da Torre de
            //Controlo que o pedido de descolagem/aterragem foi rejeitado, 
            //a thread deverá terminar.

            slot_pos = get_empty_slot();
            slot.type = ARRIVAL;
            
            msg.slot = slot_pos;
            msg.mtype = ARRIVAL;
        

            //A Torre de Controlo deverá marcar voos prioritários como urgentes e evitar que 
            //eles tenham de efetuar manobras de holding. 
            if(msg.priority==1){
                printf("prioritario...\n");
                sem_wait(sem_flight);
                sharedMemoryFlight_CT->slots[slot_pos].holding = 0;
                sem_post(sem_flight);
            }

            if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
                printf("Error sending message through message queue (%s).\n",strerror(errno));
                exit(0);
            }

            //Caso a Torre de Controlo verifique que é necessário adiar a aterragem,
            //o voo será ordenado a realizar uma manobra de holding que irá alterar o ETA

            arrival_queue_t * arrival = (arrival_queue_t*)malloc(sizeof(arrival_queue_t));
            strcpy(arrival->name,name);
            arrival->eta = eta;
            arrival->fuel = fuel;
            arrival->slot = slot_pos;
            //Deve também colocar o voo na fila de espera para
            //partidas, de acordo com o instante desejado de partida.
            append_to_queue_arrivals(arrival);
            ArrivalMergeSort(&arrival_queue);
            if(pthread_create(&arrival->thread,NULL,control_tower_arrival,(void*)arrival)){
                logger("Error creating control tower routine");
            }
        }
        if(msg.mtype == DEPARTURE){
            printf("depart\n");   
            //A torre deve guardar a informação recebida e responder pela
            //mesma MSQ com o número do slot no espaço de memória partilhada que é
            //atribuído a esse voo específico.
            char name[MAX_TEXT];
            int takeoff = msg.takeoff;
            strcpy(name, msg.name);
            slot_pos = get_empty_slot();
            printf("got slot %d\n", slot_pos); 
            slot.type = DEPARTURE;

            msg.slot = slot_pos;
            msg.mtype = DEPARTURE;

            printf("aquii\n");
            
            if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
                printf("Error sending message through message queue (%s).\n",strerror(errno));
                exit(0);
            }

            
            //Deve também colocar o voo na fila de espera para
            //partidas, de acordo com o instante desejado de partida.
            
            departure_queue_t * departure = (departure_queue_t*)malloc(sizeof(departure_queue_t));
            strcpy(departure->name,name);
            departure->takeoff = takeoff;
            departure->slot = slot_pos;
            append_to_queue_departures(departure);
            DepartureMergeSort(&departure_queue);
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

void * control_tower_arrival(void*arg){
    pthread_mutex_lock(&CT_flight_arrival_mutex);
    arrival_queue_t * arrival = (arrival_queue_t*)arg;
    while(arrival->eta!=0){
        arrival->eta--;
        arrival->fuel--;
        if(arrival->fuel == 0){
            sem_wait(sem_flight);
            sharedMemoryFlight_CT->slots[arrival->slot].detour = 1;
            log_str("Fuel:0 => Flight Detoured: %s",arrival->name);
            sem_post(sem_flight);
            pthread_mutex_unlock(&CT_flight_departure_mutex);
            pthread_exit(NULL);
        }
        msleep(1);
    }
    //Assim que receber a ordem para descolar, a thread deve tirar partido dos 
    //mecanismos de sincronização para aguardar, sem espera ativa, que uma das pistas esteja vazia, 
    //que o aeroporto possa receber descolagens e que nenhum voo esteja a aterrar. 
    //Poderá então começar o processo de descolagem, que demora T unidades de tempo.
    if(RUNWAYS[L_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
        RUNWAYS[L_01].occupied = 1;
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[arrival->slot].landing = 1;
        sem_post(sem_flight);
        log_str("Runway L01 assigned to flight %s",arrival->name);
        pthread_mutex_unlock(&CT_flight_departure_mutex);
        pthread_exit(NULL);
    }else if(RUNWAYS[R_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
        RUNWAYS[R_01].occupied = 1;
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[arrival->slot].landing = 1;
        sem_post(sem_flight);
        log_str("Runway R01 assigned to flight %s",arrival->name);
        pthread_mutex_unlock(&CT_flight_departure_mutex);
        pthread_exit(NULL);
    }else{
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[arrival->slot].holding = 1;
        sem_post(sem_flight);
        log_str("Runways for departure occupied, holding flight %s",arrival->name);
        while(arrival->fuel != 0){
            arrival->fuel--;
            if(RUNWAYS[L_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
                RUNWAYS[L_01].occupied = 1;
                sem_wait(sem_flight);
                sharedMemoryFlight_CT->slots[arrival->slot].holding = 0;
                sharedMemoryFlight_CT->slots[arrival->slot].landing = 1;
                sem_post(sem_flight);
                log_str("Runway L01 assigned to flight %s",arrival->name);
                pthread_mutex_unlock(&CT_flight_departure_mutex);
                pthread_exit(NULL);
            }else if(RUNWAYS[R_01].occupied == 0 && RUNWAYS[L_28].occupied == 0 && RUNWAYS[R_28].occupied == 0){
                RUNWAYS[R_01].occupied = 1;
                sem_wait(sem_flight);
                sharedMemoryFlight_CT->slots[arrival->slot].holding = 0;
                sharedMemoryFlight_CT->slots[arrival->slot].landing = 1;
                sem_post(sem_flight);
                log_str("Runway R01 assigned to flight %s",arrival->name);
                pthread_mutex_unlock(&CT_flight_departure_mutex);
                pthread_exit(NULL);
            }
            msleep(1);
        }
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[arrival->slot].detour = 1;
        sem_post(sem_flight);
        pthread_mutex_unlock(&CT_flight_departure_mutex);
        pthread_exit(NULL);
    }
}


void * control_tower_departure(void*arg){
    pthread_mutex_lock(&CT_flight_departure_mutex);
    departure_queue_t * departure = (departure_queue_t*)arg;
    while(departure->takeoff!=0){
        departure->takeoff--;
        msleep(1);
    }
    if(RUNWAYS[L_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
        RUNWAYS[L_28].occupied = 1;
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[departure->slot].depart = 1;
        sem_post(sem_flight);
        log_str("Runway L28 assigned to flight %s",departure->name);
        pthread_mutex_unlock(&CT_flight_departure_mutex);
        pthread_exit(NULL);
    }else if(RUNWAYS[R_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
        RUNWAYS[R_28].occupied = 1;
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[departure->slot].depart = 1;
        sem_post(sem_flight);
        log_str("Runway R28 assigned to flight %s",departure->name);
        pthread_mutex_unlock(&CT_flight_departure_mutex);
        pthread_exit(NULL);
    }else{
        int occupied = 1;
        sem_wait(sem_flight);
        sharedMemoryFlight_CT->slots[departure->slot].holding = 1;
        sem_post(sem_flight);
        while(occupied){
            if(RUNWAYS[L_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
                occupied  = 0;
                RUNWAYS[L_28].occupied = 1;
                sem_wait(sem_flight);
                sharedMemoryFlight_CT->slots[departure->slot].holding = 0;
                sharedMemoryFlight_CT->slots[departure->slot].depart = 1;
                sem_post(sem_flight);
                log_str("Runway L28 assigned to flight %s",departure->name);
                pthread_mutex_unlock(&CT_flight_departure_mutex);
                pthread_exit(NULL);
            }else if(RUNWAYS[R_28].occupied == 0 && RUNWAYS[L_01].occupied == 0 && RUNWAYS[R_01].occupied == 0){
                occupied = 0;
                RUNWAYS[R_28].occupied = 1;
                sem_wait(sem_flight);
                sharedMemoryFlight_CT->slots[departure->slot].holding = 0;
                sharedMemoryFlight_CT->slots[departure->slot].depart = 1;
                sem_post(sem_flight);
                log_str("Runway R28 assigned to flight %s",departure->name);
                pthread_mutex_unlock(&CT_flight_departure_mutex);
                pthread_exit(NULL);
            }
            msleep(1);
        }
    }
    pthread_mutex_unlock(&CT_flight_departure_mutex);
    pthread_exit(NULL);
}

