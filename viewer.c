#include <stdio.h>
#include <time.h>
#include <unistd.h>   // sleep()
#include <stdint.h>

// Ajusta esto según la estructura real que use viewer.c:
typedef struct {
    char ip[64];
    float cpu_total;
    float cpu_user;
    float cpu_sys;
    float cpu_idle;
    uint64_t mem_used_kb;
    uint64_t mem_free_kb;
    time_t last_update; // epoch seconds when this entry was updated
    int valid; // opcional: 1 si la entrada está llena
} host_stats_t;

// Umbral para considerar datos "recientes" (segundos)
#define STALE_THRESHOLD 5

// Imprime la tabla. 'hosts' y 'nhosts' deben venir desde la estructura compartida que ya usas.
void print_table(host_stats_t *hosts, int nhosts) {
    time_t now = time(NULL);

    // Limpiar pantalla
    printf("\033[2J\033[H");

    // Cabecera
    printf("%-15s %6s %10s %9s %9s %12s %12s\n",
           "IP", "CPU%", "CPU_user%", "CPU_sys%", "CPU_idle%", "Mem_used_MB", "Mem_free_MB");

    for (int i = 0; i < nhosts; ++i) {
        host_stats_t *h = &hosts[i];

        // Si tienes un campo 'valid' puedes usarlo; de lo contrario usa last_update
        int is_fresh = (h->valid && (now - h->last_update) <= STALE_THRESHOLD)
                       || ((now - h->last_update) <= STALE_THRESHOLD);

        if (!is_fresh) {
            // Mostrar IP y columnas marcadas como sin datos
            printf("%-15s %6s %10s %9s %9s %12s %12s\n",
                   h->ip,
                   "--", "--", "--", "--",
                   "--", "--");
        } else {
            double mem_used_mb = h->mem_used_kb / 1024.0;
            double mem_free_mb = h->mem_free_kb / 1024.0;
            printf("%-15s %6.1f %10.1f %9.1f %9.1f %12.0f %12.0f\n",
                   h->ip,
                   h->cpu_total,
                   h->cpu_user,
                   h->cpu_sys,
                   h->cpu_idle,
                   mem_used_mb,
                   mem_free_mb);
        }
    }

    fflush(stdout);
}
