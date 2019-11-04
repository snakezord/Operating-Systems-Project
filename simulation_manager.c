#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shared.h"
#include "simulation_manager.h"

int read_config_file(settings_t*settings){
    FILE * file;
    file = fopen(FILENAME, "r");
    if(file==NULL){
        return -1;
    }
    char line[MAX_TEXT];
    if(fscanf(file,"%d",&(settings->time_unit))!=1){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d, %d",&(settings->takeoff_duration),&(settings->takeoff_interval))!=2){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d, %d",&(settings->landing_duration),&(settings->landing_interval))!=2){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d, %d",&(settings->holding_min_duration),&(settings->holding_max_duration))!=2){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d",&(settings->max_departures_on_system))!=1){
        fclose(file);
        return -1;
    }
    fgets(line,MAX_TEXT,file);
    if(fscanf(file,"%d",&(settings->max_arrivals_on_system))!=1){
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}