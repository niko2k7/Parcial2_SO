#ifndef VIEWER_SHARED_DATA_H
#define VIEWER_SHARED_DATA_H

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "config.h"

// Estructura de información de un host (debe coincidir con collector)
typedef struct {
    char ip[IP_LENGTH];
    float cpu_usage;
    float cpu_user;
    float cpu_system;
    float cpu_idle;
    float mem_used_mb;
    float mem_free_mb;
    float swap_total_mb;
    float swap_free_mb;
    time_t last_update;
    int active;
} host_info_t;

// Datos compartidos (debe coincidir con collector)
typedef struct {
    host_info_t hosts[MAX_HOSTS];
    int num_hosts;
} shared_data_t;

// Funciones de memoria compartida
void* attach_shared_memory(int shmid);
void detach_shared_memory(void *shmaddr);

// Funciones de semáforos
void sem_wait(int semid);
void sem_signal(int semid);

#endif // VIEWER_SHARED_DATA_H
