#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "config.h"

// Tipos de mensajes
typedef enum {
    MSG_CPU,
    MSG_MEM,
    MSG_UNKNOWN
} msg_type_t;

// Información de un host
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

// Datos compartidos entre collector y viewer
typedef struct {
    host_info_t hosts[MAX_HOSTS];
    int num_hosts;
} shared_data_t;

// Funciones de memoria compartida y semáforos
int init_shared_memory(key_t key, size_t size);
void* attach_shared_memory(int shmid);
void detach_shared_memory(void *shmaddr);
void destroy_shared_memory(int shmid);

int init_semaphore(key_t key);
void sem_wait(int semid);
void sem_signal(int semid);
void destroy_semaphore(int semid);

#endif // SHARED_DATA_H
