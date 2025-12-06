#include "shared_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Union para operaciones de semáforo
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// ============================================================================
// FUNCIONES DE MEMORIA COMPARTIDA
// ============================================================================

int init_shared_memory(key_t key, size_t size) {
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return -1;
    }
    return shmid;
}

void* attach_shared_memory(int shmid) {
    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void*)-1) {
        perror("shmat");
        return NULL;
    }
    return shmaddr;
}

void detach_shared_memory(void *shmaddr) {
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
    }
}

void destroy_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }
}

// ============================================================================
// FUNCIONES DE SEMÁFOROS
// ============================================================================

int init_semaphore(key_t key) {
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return -1;
    }
    
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl");
        return -1;
    }
    
    return semid;
}

void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop wait");
    }
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop signal");
    }
}

void destroy_semaphore(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl destroy");
    }
}
