#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "config.h"
#include "shared_data.h"

void clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_header() {
    printf("╔═══════════════╦═══════╦═══════════╦══════════╦═══════════╦═════════════╦═════════════╗\n");
    printf("║ IP            ║ CPU%%  ║ CPU_user%% ║ CPU_sys%% ║ CPU_idle%% ║ Mem_used_MB ║ Mem_free_MB ║\n");
    printf("╠═══════════════╬═══════╬═══════════╬══════════╬═══════════╬═════════════╬═════════════╣\n");
}

void print_footer() {
    printf("╚═══════════════╩═══════╩═══════════╩══════════╩═══════════╩═════════════╩═════════════╝\n");
}

void print_host_row(host_info_t *host) {
    time_t now = time(NULL);
    int is_active = (now - host->last_update) < INACTIVE_TIMEOUT && host->active;
    
    if (is_active) {
        printf("║ %-13s ║ %5.1f ║     %5.1f ║    %5.1f ║     %5.1f ║   %9.2f ║   %9.2f ║\n",
               host->ip,
               host->cpu_usage,
               host->cpu_user,
               host->cpu_system,
               host->cpu_idle,
               host->mem_used_mb,
               host->mem_free_mb);
    } else {
        printf("║ %-13s ║    -- ║        -- ║       -- ║        -- ║          -- ║          -- ║\n",
               host->ip);
    }
}

void display_table(shared_data_t *data, int semid) {
    clear_screen();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    MONITOR DISTRIBUIDO - Vista en Tiempo Real                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Adquirir semáforo para leer
    sem_wait(semid);
    
    print_header();
    
    if (data->num_hosts == 0) {
        printf("║ Sin datos     ║    -- ║        -- ║       -- ║        -- ║          -- ║          -- ║\n");
    } else {
        for (int i = 0; i < data->num_hosts && i < MAX_HOSTS; i++) {
            print_host_row(&data->hosts[i]);
        }
    }
    
    print_footer();
    
    // Liberar semáforo
    sem_signal(semid);
    
    // Mostrar timestamp
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';  // Eliminar \n
    printf("\nÚltima actualización: %s\n", time_str);
    printf("Presiona Ctrl+C para salir\n");
}

int main() {
    printf("Iniciando Viewer...\n");
    
    // Obtener memoria compartida (debe existir, creada por collector)
    int shmid = shmget(SHM_KEY, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        fprintf(stderr, "Error: No se pudo acceder a la memoria compartida.\n");
        fprintf(stderr, "Asegúrate de que el collector esté ejecutándose.\n");
        return EXIT_FAILURE;
    }
    
    // Adjuntar memoria compartida
    shared_data_t *shared_data = (shared_data_t *)attach_shared_memory(shmid);
    if (shared_data == NULL) {
        fprintf(stderr, "Error: No se pudo adjuntar memoria compartida\n");
        return EXIT_FAILURE;
    }
    
    // Obtener semáforo
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("semget");
        detach_shared_memory(shared_data);
        fprintf(stderr, "Error: No se pudo acceder al semáforo.\n");
        fprintf(stderr, "Asegúrate de que el collector esté ejecutándose.\n");
        return EXIT_FAILURE;
    }
    
    printf("Conectado a memoria compartida (0x%04X)\n", SHM_KEY);
    printf("Actualizando cada %d segundos...\n\n", REFRESH_INTERVAL);
    sleep(1);
    
    // Loop principal: mostrar tabla cada REFRESH_INTERVAL segundos
    while (1) {
        display_table(shared_data, semid);
        sleep(REFRESH_INTERVAL);
    }
    
    // Limpieza (nunca se alcanza en operación normal)
    detach_shared_memory(shared_data);
    
    return EXIT_SUCCESS;
}
