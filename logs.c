#include "shared.h"

FILE * logs_file;

int init_logs(){
    logs_file = fopen("logs.txt","w");
    if(logs_file==NULL){
        return -1;
    }
    if(sem_log==NULL){
        fclose(logs_file);
        return -1;
    }
    return 0;
}

void logger(const char*string){
    char buffer[100];
    struct  tm *sTm;
    time_t now = time(0);
    sTm = gmtime(&now);
    strftime(buffer,sizeof(buffer), "%H:%M:%S", sTm);
    sem_wait(sem_log);
    printf("%s %s", buffer, string);
    logs_file = fopen("logs.txt","a");
    if(logs_file != NULL){
        fprintf(logs_file,"%s %s", buffer, string);
        fclose(logs_file);
    }
    sem_post(sem_log);
}

void log_int(const char*string, int n){
    char buffer[100];
    sprintf(buffer,string,n);
    logger(buffer);
}

void log_str(const char*str, char*s){
    char buffer[100];
    sprintf(buffer,str,s);
    logger(buffer);
}
