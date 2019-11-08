#include <semaphore.h>

#include "shared.h"

#define SEMAPHORE_NAME "logs_sem"

sem_t *sem_logs;
FILE * logs_file;

int init_logs(){
    logs_file = fopen("logs.txt","w");
    if(logs_file==NULL){
        return -1;
    }
    sem_unlink(SEMAPHORE_NAME);
    sem_logs = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 1);
    if(sem_logs==NULL){
        fclose(logs_file);
        return -1;
    }
    return 0;
}

void log(const char*string){
    char buffer[100];
    struct  tm *sTm;
    time_t now = time(0);
    sTm = gmtime(&now);
    strftime(buffer,sizeof(buffer), "%H:%M:%S", sTm);
    logs_file = fopen("logs.txt","a");
    if(logs_file != NULL){
        fprintf(logs_file,"%s %s", buffer, string);
        fclose(logs_file);
    }
    sem_post(sem_logs);
}

void log_int(const char*string, int n){
    char buffer[100];
    sprintf(buffer,string,n);
    log(buffer);
}

void log_str(const char*str, char*s){
    char buffer[100];
    sprintf(buffer,str,s);
    log(buffer);
}

void log_close(){
    fclose(logs_file);
    sem_close(sem_logs);
}