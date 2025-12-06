#include "shared_data.h"
#include <stdio.h>
#include <stdlib.h>

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
