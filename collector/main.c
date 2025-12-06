#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "config.h"
#include "shared_data.h"
#include "network.h"

// Variables globales para limpieza
int shmid_global = -1;
int semid_global = -1;
int server_fd_global = -1;

void cleanup_handler(int signum) {
    (void)signum;  // Marcar como no usado
    printf("\nLimpiando recursos...\n");
    
    if (server_fd_global != -1) {
        close(server_fd_global);
    }
    if (semid_global != -1) {
        destroy_semaphore(semid_global);
    }
    if (shmid_global != -1) {
        destroy_shared_memory(shmid_global);
    }
    
    printf("Collector terminado\n");
    exit(0);
}

void *display_thread(void *arg) {
    shared_data_t *shared_data = (shared_data_t *)arg;
    
    while (1) {
        // Limpiar pantalla
        printf("\033[2J\033[H");
        
        // Imprimir encabezado
        printf("%-15s %-8s %-11s %-11s %-11s %-13s %-13s\n", 
               "IP", "CPU%", "CPU_user%", "CPU_sys%", "CPU_idle%", "Mem_used_MB", "Mem_free_MB");
        
        sem_wait(semid_global);
        
        for (int i = 0; i < shared_data->num_hosts; i++) {
             host_info_t *host = &shared_data->hosts[i];
             time_t now = time(NULL);
             
             // Verificar si el host tiene datos recientes
             if (now - host->last_update > UPDATE_TIMEOUT) { // Usamos el timeout de config
                 printf("%-15s %-8s %-11s %-11s %-11s %-13s %-13s\n", 
                        host->ip, "--", "--", "--", "--", "--", "--");
             } else {
                 printf("%-15s %-8.1f %-11.1f %-11.1f %-11.1f %-13.0f %-13.0f\n",
                    host->ip,
                    host->cpu_usage,
                    host->cpu_user,
                    host->cpu_system,
                    host->cpu_idle,
                    host->mem_used_mb,
                    host->mem_free_mb);
             }
        }
        
        sem_signal(semid_global);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Puerto inválido\n");
        return EXIT_FAILURE;
    }
    
    printf("Iniciando Collector...\n");
    
    // Configurar señales
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);
    
    // Inicializar memoria compartida
    shmid_global = init_shared_memory(SHM_KEY, sizeof(shared_data_t));
    if (shmid_global == -1) {
        fprintf(stderr, "Error: No se pudo crear memoria compartida\n");
        return EXIT_FAILURE;
    }
    
    shared_data_t *shared_data = attach_shared_memory(shmid_global);
    if (shared_data == NULL) {
        destroy_shared_memory(shmid_global);
        fprintf(stderr, "Error: No se pudo adjuntar memoria compartida\n");
        return EXIT_FAILURE;
    }
    
    memset(shared_data, 0, sizeof(shared_data_t));
    
    // Inicializar semáforo
    semid_global = init_semaphore(SEM_KEY);
    if (semid_global == -1) {
        detach_shared_memory(shared_data);
        destroy_shared_memory(shmid_global);
        fprintf(stderr, "Error: No se pudo crear semáforo\n");
        return EXIT_FAILURE;
    }
    
    // Crear socket servidor
    server_fd_global = create_server_socket(port);
    if (server_fd_global == -1) {
        detach_shared_memory(shared_data);
        destroy_shared_memory(shmid_global);
        destroy_semaphore(semid_global);
        fprintf(stderr, "Error: No se pudo crear socket servidor\n");
        return EXIT_FAILURE;
    }
    
    // Start display thread
    pthread_t display_tid;
    if (pthread_create(&display_tid, NULL, display_thread, shared_data) != 0) {
        perror("pthread_create display");
    }
    pthread_detach(display_tid);

    
    // Loop principal: aceptar conexiones
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_global, 
                              (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        
        // Crear argumentos para el thread
        client_args_t *args = malloc(sizeof(client_args_t));
        if (args == NULL) {
            close(client_fd);
            continue;
        }
        
        args->client_fd = client_fd;
        args->shared_data = shared_data;
        args->semid = semid_global;
        
        // Crear thread para manejar cliente
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, args) != 0) {
            perror("pthread_create");
            free(args);
            close(client_fd);
            continue;
        }
        
        pthread_detach(thread_id);
    }
    
    return EXIT_SUCCESS;
}
