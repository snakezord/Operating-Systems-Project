#make: simulation_manager.c control_tower.c shared.c
gcc simulation_manager.c control_tower.c shared.c -D_REENTRANT -Wall -Wno-deprecated -lpthread -lm -o main
