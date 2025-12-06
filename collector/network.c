#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ============================================================================
// FUNCIONES DE UTILIDAD
// ============================================================================

void log_message(const char *level, const char *message) {
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    printf("[%s] [%s] %s\n", time_str, level, message);
    fflush(stdout);
}

// ============================================================================
// FUNCIONES DE RED
// ============================================================================

int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return -1;
    }
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return -1;
    }
    
    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "Servidor escuchando en puerto %d", port);
    log_message("INFO", log_msg);
    
    return server_fd;
}

// ============================================================================
// FUNCIONES DE PARSEO
// ============================================================================

msg_type_t parse_message(const char *msg, char *ip, float *metrics) {
    char type[4];
    
    if (sscanf(msg, "%3[^;];%31[^;];", type, ip) != 2) {
        return MSG_UNKNOWN;
    }
    
    if (strcmp(type, "CPU") == 0) {
        if (sscanf(msg, "CPU;%*[^;];%f;%f;%f;%f", 
                   &metrics[0], &metrics[1], &metrics[2], &metrics[3]) == 4) {
            return MSG_CPU;
        }
    } else if (strcmp(type, "MEM") == 0) {
        if (sscanf(msg, "MEM;%*[^;];%f;%f;%f;%f", 
                   &metrics[0], &metrics[1], &metrics[2], &metrics[3]) == 4) {
            return MSG_MEM;
        }
    }
    
    return MSG_UNKNOWN;
}

// ============================================================================
// FUNCIONES DE ALMACENAMIENTO
// ============================================================================

int find_or_create_host(shared_data_t *data, const char *ip) {
    // Buscar host existente
    for (int i = 0; i < data->num_hosts; i++) {
        if (strcmp(data->hosts[i].ip, ip) == 0) {
            data->hosts[i].active = 1;
            return i;
        }
    }
    

    // Reutilizar un host inactivo
    for (int i = 0; i < data->num_hosts; i++) {
        if (data->hosts[i].active == 0) {
            strncpy(data->hosts[i].ip, ip, IP_LENGTH - 1);
            data->hosts[i].active = 1;
            data->hosts[i].last_update = time(NULL);
            return i;
        }
    }

    // Crear nuevo host si hay espacio
    if (data->num_hosts < MAX_HOSTS) {
        int idx = data->num_hosts;
        strncpy(data->hosts[idx].ip, ip, IP_LENGTH - 1);
        data->hosts[idx].ip[IP_LENGTH - 1] = '\0';
        data->hosts[idx].active = 1;
        data->num_hosts++;
        
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Nuevo host: %s", ip);
        log_message("INFO", log_msg);
        
        return idx;
    }
    
    return -1;
}

void update_host_data(shared_data_t *data, int semid, const char *ip, 
                      msg_type_t type, const float *metrics) {
    sem_wait(semid);
    
    int idx = find_or_create_host(data, ip);
    if (idx != -1) {
        host_info_t *host = &data->hosts[idx];
        
        if (type == MSG_CPU) {
            host->cpu_usage = metrics[0];
            host->cpu_user = metrics[1];
            host->cpu_system = metrics[2];
            host->cpu_idle = metrics[3];
        } else if (type == MSG_MEM) {
            host->mem_used_mb = metrics[0];
            host->mem_free_mb = metrics[1];
            host->swap_total_mb = metrics[2];
            host->swap_free_mb = metrics[3];
        }
        
        host->last_update = time(NULL);
        host->active = 1;
    }
    
    sem_signal(semid);
}

// ============================================================================
// MANEJO DE CLIENTES
// ============================================================================

void* handle_client(void *arg) {
    client_args_t *args = (client_args_t *)arg;
    int client_fd = args->client_fd;
    shared_data_t *shared_data = args->shared_data;
    int semid = args->semid;
    
    char buffer[BUFFER_SIZE];
    char ip[IP_LENGTH];
    float metrics[4];
    
    log_message("INFO", "Cliente conectado");
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes <= 0) {

            log_message("INFO", "Cliente desconectado");

            // buscamos el cliente que se desconecta, cambiamos la flag de actividad y seteamos el ultimo cambio a cero.
            for (int i = 0; i < shared_data->num_hosts; i++) {
                if (strcmp(shared_data->hosts[i].ip, ip) == 0) {
                    shared_data->hosts[i].active = 0;
                    shared_data->hosts[i].last_update = 0;

                }
            }
            break;
        }
        
        buffer[bytes] = '\0';
        msg_type_t type = parse_message(buffer, ip, metrics);
        
        if (type != MSG_UNKNOWN) {
            update_host_data(shared_data, semid, ip, type, metrics);
            
            /*
            char log_msg[256];
            if (type == MSG_CPU) {
                snprintf(log_msg, sizeof(log_msg), 
                         "CPU %s: %.1f%%", ip, metrics[0]);
            } else {
                snprintf(log_msg, sizeof(log_msg), 
                         "MEM %s: %.1fMB usado", ip, metrics[0]);
            }
            log_message("INFO", log_msg);
            */
        }
    }
    
    close(client_fd);
    free(args);
    return NULL;
}
