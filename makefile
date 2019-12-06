#make: simulation_manager.c control_tower.c shared.c
gcc -g simulation_manager.c control_tower.c shared.c logs.c flight.c -D_REENTRANT -Wall -Wno-deprecated -lpthread -lm -o main
