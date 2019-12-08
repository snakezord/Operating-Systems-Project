#include "shared.h"



void * flight_arrival(void*arg){
    pthread_mutex_lock(&flight_arrival_mutex);
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
    //Caso um voo de chegada apenas tenha combustível para voar até ao instante ‘4 +
    //ETA + duração da aterragem’, a mensagem a enviar à Torre de Controlo através
    //da fila de mensagens deve ser prioritária e lida antes das outras.
    //A thread deverá escrever no log o instante em que enviou a mensagem de emergência
    if(arrival->fuel <= 4+(arrival->eta)+settings.landing_duration){
        msg.mtype = PRIORITY;
        msg.priority = 1;
    }
    if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
        printf("Error sending message through message queue (%s).\n",strerror(errno));
        exit(0);
    }
    
    if(msgrcv(msqid,&msg,sizeof(msg),ARRIVAL,0) < 0){
            logger("Error receiving messagem from message queue 1.\n");
            exit(0); 
    }
    int slot = msg.slot;
    
    //As threads correspondentes às chegadas recebem as instruções da Torre de
    //Controlo através da memória partilhada - aterragem, holding, desvio para outro aeroporto.
    sem_wait(sem_flight);
    if(sharedMemoryFlight_CT->slots[slot].landing == 1){
        sem_post(sem_flight);
        //Os voos de chegada demoram L unidades de tempo a aterrar
        msleep(settings.landing_duration);
        log_str("Flight landed: %s", arrival->name);

    }
    sem_wait(sem_flight);
    if(sharedMemoryFlight_CT->slots[slot].holding == 1){
        sem_post(sem_flight);
        log_str("Flight on holding: %s", arrival->name);

    }
    sem_wait(sem_flight);
    if(sharedMemoryFlight_CT->slots[slot].detour == 1){
        sem_post(sem_flight);
        log_str("Flight detoured completed: %s", arrival->name);
        pthread_mutex_unlock(&flight_arrival_mutex);
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&flight_arrival_mutex);
    pthread_exit(NULL);
}

void * flight_departure(void*arg){
    
    pthread_mutex_lock(&flight_departure_mutex);
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
    printf("antes de mandar em depart\n");
    if(msgsnd(msqid,&msg,sizeof(msg),0) < 0){
        printf("Error sending message through message queue (%s).\n",strerror(errno));
        exit(0);
    }

    if(msgrcv(msqid,&msg,sizeof(msg),DEPARTURE,0) < 0){
            logger("Error receiving messagem from message queue 3.\n");
            exit(0);
    }
    
    int slot = msg.slot;
    //As threads correspondentes às partidas recebem a indicação de descolagem pelo
    //seu slot em memória partilhada.
    sem_wait(sem_flight);
    if(sharedMemoryFlight_CT->slots[slot].depart == 1){
        sem_post(sem_flight);
        pthread_mutex_unlock(&flight_departure_mutex);
        pthread_exit(NULL);
    }
    sem_wait(sem_flight);
    if(sharedMemoryFlight_CT->slots[slot].holding == 1){
        sem_post(sem_flight);
        //falta
    }
    pthread_mutex_unlock(&flight_departure_mutex);
    pthread_exit(NULL);
}