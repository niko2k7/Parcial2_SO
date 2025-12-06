#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 3535

// struct host_info{
//     // Cpu
//     char ip[32];
//     float cpu_usage;
//     float cpu_user;
//     float cpu_system;
//     float cpu_idle;
//     // Memoria
//     float swap_total_mb;
//     float swap_free_mb;
//     float mem_total_mb;
//     float mem_used_mb; // Total - Free
//     float mem_free_mb;
    
// };

void getMemInfo(char *buffer){
    
    FILE *f = fopen("/proc/meminfo", "r");

    if (!f) {
        perror("fopen");
        return;
    }

    char line[256];
    long mem_total_kb = 0, mem_available_kb = 0, mem_free_kb = 0, swap_total_kb = 0, swap_free_kb = 0;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "MemTotal: %ld kB", &mem_total_kb) == 1) continue;
        if (sscanf(line, "MemAvailable: %ld kB", &mem_available_kb) == 1) continue;
        if (sscanf(line, "MemFree: %ld kB", &mem_free_kb) == 1) continue;
        if (sscanf(line, "SwapTotal: %ld kB", &swap_total_kb) == 1) continue;
        if (sscanf(line, "SwapFree: %ld kB", &swap_free_kb) == 1) continue;
    }
    fclose(f);

    // Convertir memoria de KB a MB
    float mem_total_mb = mem_total_kb / 1024.0f;
    float mem_available_mb = mem_available_kb / 1024.0f;
    float mem_free_mb = mem_free_kb / 1024.0f;
    float mem_used_mb = (mem_total_kb - mem_available_kb) / 1024.0f;
    // Convertir swap de KB a MB
    float swap_total_mb = swap_total_kb / 1024.0f;
    float swap_free_mb = swap_free_kb / 1024.0f;

    // MEM;<ip_logica_agente>;<mem_used_MB>;<MemFree_MB>;<SwapTotal_MB>;<SwapFree_MB>\n
    snprintf(buffer, 256, "MEM;%s;%.2f;%.2f;%.2f;%.2f\n",
            "IP",
            mem_used_mb,
            mem_free_mb,
            swap_total_mb,
            swap_free_mb);


    return;
}


void getCpuInfo(char *buffer){
    FILE *f = fopen("/proc/stat", "r");
    if (!f) {
        perror("fopen");
        return;
    }

    char cpu_label[5];
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
    if (fscanf(f, "%4s %lu %lu %lu %lu %lu %lu %lu %lu", cpu_label, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) == 9) {
        // printf("Label: %s\n", cpu_label);
        // printf("user=%lu nice=%lu system=%lu idle=%lu\n", user, nice, system, idle);
    } else {
        fprintf(stderr, "No se pudo leer la línea de cpu\n");
    }
    fclose(f);

    return;
}


int connectToViewer(){

}


// ./monitor    ip_recolector  puerto   ip_máquina
// argv[1] ip_recolector
// argv[2] puerto
// argv[3] ip_lógica_máquina
int main(int argc, char *argv[]) {
    struct sockaddr_in client;
    char buffer[256];

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("Error al crear socket");
        exit(-1);
    }

    client.sin_family = AF_INET;
    client.sin_port = htons(argv[2]);
    client.sin_addr.s_addr = inet_addr(argv[1]);
    bzero(&client.sin_zero, 8);

    int r = connect(fd, (struct sockaddr *)&client, sizeof(struct sockaddr_in));
    // Validar error de conexión
    if(r == -1) perror("Error client connect");

    r = recv(fd, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    getMemInfo(buffer);
    r = send(fd, buffer, strlen(buffer), 0);



    

    //getCpuInfo(&buffer);

    close(fd);

    return 0;
}



