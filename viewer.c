#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

// Configuración (debe coincidir con collector)
#define MAX_HOSTS 4
#define IP_LENGTH 32
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define REFRESH_INTERVAL 2
#define INACTIVE_TIMEOUT 10

// Estructuras (deben coincidir con collector)
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

typedef struct {
    host_info_t hosts[MAX_HOSTS];
    int num_hosts;
} shared_data_t;

void sem_wait_op(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal_op(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

void clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void display_table(shared_data_t *data, int semid) {
    clear_screen();
    
    printf("\n╔════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    MONITOR DISTRIBUIDO - Vista en Tiempo Real                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════════════╝\n\n");
    
    printf("╔═══════════════╦═══════╦═══════════╦══════════╦═══════════╦═════════════╦═════════════╗\n");
    printf("║ IP            ║ CPU%%  ║ CPU_user%% ║ CPU_sys%% ║ CPU_idle%% ║ Mem_used_MB ║ Mem_free_MB ║\n");
    printf("╠═══════════════╬═══════╬═══════════╬══════════╬═══════════╬═════════════╬═════════════╣\n");
    
    sem_wait_op(semid);
    
    if (data->num_hosts == 0) {
        printf("║ Sin datos     ║    -- ║        -- ║       -- ║        -- ║          -- ║          -- ║\n");
    } else {
        time_t now = time(NULL);
        for (int i = 0; i < data->num_hosts && i < MAX_HOSTS; i++) {
            host_info_t *host = &data->hosts[i];
            int is_active = (now - host->last_update) < INACTIVE_TIMEOUT && host->active;
            
            if (is_active) {
                printf("║ %-13s ║ %5.1f ║     %5.1f ║    %5.1f ║     %5.1f ║   %9.2f ║   %9.2f ║\n",
                       host->ip, host->cpu_usage, host->cpu_user, host->cpu_system,
                       host->cpu_idle, host->mem_used_mb, host->mem_free_mb);
            } else {
                printf("║ %-13s ║    -- ║        -- ║       -- ║        -- ║          -- ║          -- ║\n",
                       host->ip);
            }
        }
    }
    
    sem_signal_op(semid);
    
    printf("╚═══════════════╩═══════╩═══════════╩══════════╩═══════════╩═════════════╩═════════════╝\n");
    
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    printf("\nÚltima actualización: %s\n", time_str);
    printf("Presiona Ctrl+C para salir\n");
}

int main() {
    printf("Iniciando Viewer...\n");
    
    // Obtener memoria compartida
    int shmid = shmget(SHM_KEY, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        fprintf(stderr, "Error: No se pudo acceder a la memoria compartida.\n");
        fprintf(stderr, "Asegúrate de que el collector esté ejecutándose.\n");
        return 1;
    }
    
    shared_data_t *shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat");
        return 1;
    }
    
    // Obtener semáforo
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("semget");
        shmdt(shared_data);
        fprintf(stderr, "Error: No se pudo acceder al semáforo.\n");
        return 1;
    }
    
    printf("Conectado a memoria compartida (0x%04X)\n", SHM_KEY);
    printf("Actualizando cada %d segundos...\n\n", REFRESH_INTERVAL);
    sleep(1);
    
    while (1) {
        display_table(shared_data, semid);
        sleep(REFRESH_INTERVAL);
    }
    
    shmdt(shared_data);
    return 0;
}