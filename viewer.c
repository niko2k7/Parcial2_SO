#include <stdio.h>
#include <time.h>
#include <unistd.h>     // sleep()
#include <stdint.h>
#include <string.h>     // strncpy, memset
#include <stdlib.h>     // calloc, free, rand
#include <signal.h>     // signal, SIGINT

// Estructura de datos (debe coincidir con la usada por el Recolector)
typedef struct {
    char ip[64];
    float cpu_total;
    float cpu_user;
    float cpu_sys;
    float cpu_idle;
    uint64_t mem_used_kb;
    uint64_t mem_free_kb;
    time_t last_update; // epoch seconds when this entry was updated
    int valid; // 1 si la entrada ha sido llenada con datos
} host_stats_t;

// Umbral para considerar datos "recientes" (segundos)
#define STALE_THRESHOLD 5
#define REFRESH_INTERVAL_DEFAULT 2
#define MAX_HOSTS 4 // Máximo de hosts a simular/monitorear

static volatile int keep_running = 1;

// Manejo de Ctrl+C (SIGINT) para salir del bucle principal limpiamente
void handle_sigint(int sig) {
    (void)sig; // Evita warning de parámetro no usado
    keep_running = 0;
}

// Imprime la tabla. 'hosts' y 'nhosts' deben venir desde la estructura compartida.
void print_table(host_stats_t *hosts, int nhosts) {
    time_t now = time(NULL);

    // Limpiar pantalla (\033[2J\033[H es una secuencia de escape ANSI portable)
    printf("\033[2J\033[H");

    // Cabecera
    printf("%-15s %6s %10s %9s %9s %12s %12s\n",
           "IP", "CPU%", "CPU_user%", "CPU_sys%", "CPU_idle%", "Mem_used_MB", "Mem_free_MB");

    for (int i = 0; i < nhosts; ++i) {
        host_stats_t *h = &hosts[i];

        // Determinar si los datos son frescos o si nunca han sido inicializados
        int is_fresh = (h->valid && (now - h->last_update) <= STALE_THRESHOLD);

        // Si la IP no está inicializada o los datos son viejos
        if (!is_fresh) {
            // Mostrar IP y columnas marcadas como sin datos ("--")
            const char *ip_display = (h->ip[0] != '\0') ? h->ip : "Esperando IP";
            printf("%-15s %6s %10s %9s %9s %12s %12s\n",
                   ip_display,
                   "--", "--", "--", "--",
                   "--", "--");
        } else {
            // Mostrar datos válidos
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

    // Asegura que la salida se muestre inmediatamente
    fflush(stdout);
}

// Función demo: rellena algunos hosts de ejemplo para la fase de desarrollo.
void populate_demo(host_stats_t *hosts, int nhosts) {
    time_t now = time(NULL);

    const char *ips[] = { "10.0.0.1", "10.0.0.2", "10.0.0.3", "10.0.0.4" };
    for (int i = 0; i < nhosts && i < MAX_HOSTS; ++i) {
        // Inicializar a cero para limpieza
        memset(&hosts[i], 0, sizeof(host_stats_t)); 
        
        // Asignar IP y datos iniciales
        strncpy(hosts[i].ip, ips[i], sizeof(hosts[i].ip)-1);
        hosts[i].cpu_total = 20.0f + (i * 10.0f);
        hosts[i].cpu_user = hosts[i].cpu_total * 0.7f;
        hosts[i].cpu_sys = hosts[i].cpu_total * 0.3f;
        hosts[i].cpu_idle = 100.0f - hosts[i].cpu_total;
        hosts[i].mem_used_kb = (2048ULL * 1024ULL) + (i * 512ULL * 1024ULL);
        hosts[i].mem_free_kb = (1024ULL * 1024ULL) - (i * 256ULL * 1024ULL);
        hosts[i].last_update = now;
        hosts[i].valid = 1;
    }

    // Simular que la tercera IP no tiene datos recientes (mostrará "--")
    if (nhosts >= 3) {
        hosts[2].last_update = now - (STALE_THRESHOLD + 10);
    }
}

// Lógica principal: Orquesta el visualizador
int main(int argc, char **argv) {
    // Modo demo por defecto (para desarrollo). Si quieres usar la SHM, puedes pasar un argumento.
    int demo_mode = 1;
    int nhosts = MAX_HOSTS;
    int refresh_interval = REFRESH_INTERVAL_DEFAULT;

    // Aquí se manejaría la lógica de argumentos si quieres cambiar nhosts o el modo.
    // ...

    // 1. Configurar la salida limpia con Ctrl+C
    signal(SIGINT, handle_sigint);

    // 2. Asignar memoria para los hosts (o mapear la memoria compartida)
    host_stats_t *hosts = calloc(nhosts, sizeof(host_stats_t));
    if (!hosts) {
        perror("calloc");
        return 1;
    }

    // 3. Inicializar datos (aquí iría el mapeo de SHM en el modo "live")
    if (demo_mode) {
        populate_demo(hosts, nhosts);
    } else {
        // Puntos de integración: Aquí irían las llamadas a shmget, shmat, etc., 
        // para mapear la memoria compartida con el recolector.
    }

    // 4. Bucle principal: refrescar tabla periódicamente
    while (keep_running) {
        // En modo demo, podemos simular cambios para que la tabla se actualice
        if (demo_mode) {
             // Lógica de simulación para hacer que los valores cambien
             // (No implementada aquí para no depender de rand() y la librería -lm)
        } else {
            // En modo live, aquí se leería la SHM y se esperarían los semáforos.
        }

        print_table(hosts, nhosts);
        sleep(refresh_interval); // Esperar 2 segundos
    }

    // 5. Limpieza y salida
    free(hosts);
    printf("\nVisualizador terminado.\n");
    return 0;
}
