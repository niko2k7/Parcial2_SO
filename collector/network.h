#ifndef NETWORK_H
#define NETWORK_H

#include "shared_data.h"

// Argumentos para threads de clientes
typedef struct {
    int client_fd;
    shared_data_t *shared_data;
    int semid;
} client_args_t;

// Funciones de red
int create_server_socket(int port);
void* handle_client(void *arg);

// Funciones de procesamiento
msg_type_t parse_message(const char *msg, char *ip, float *metrics);
void update_host_data(shared_data_t *data, int semid, const char *ip, 
                      msg_type_t type, const float *metrics);

#endif // NETWORK_H
