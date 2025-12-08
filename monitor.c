#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


void getMemInfo(char *buffer, char *ip_logica){
    
    // abrimos el archivo con datos de memoria
    // funcion memoria ---------------------------------------------------------------------------
    FILE *f = fopen("/proc/meminfo", "r");

    if (!f) {
        perror("Error al abrir /proc/meminfo");
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
            ip_logica,
            mem_used_mb,
            mem_free_mb,
            swap_total_mb,
            swap_free_mb);

    return;
}
    //--------------------------------------------------------------------------------------------------




    // funcion cpu -------------------------------------------------------------------------------------
void getCpuInfo(char *buffer, char *ip_logica) {
    
    // 1. Variables estáticas: Conservan su valor entre llamadas para calcular el "delta"
    static unsigned long p_user = 0, p_nice = 0, p_system = 0, p_idle = 0;
    static unsigned long p_iowait = 0, p_irq = 0, p_softirq = 0, p_steal = 0;

    FILE *f = fopen("/proc/stat", "r");
    if (!f) {
        perror("fopen /proc/stat");
        return;
    }

    char cpu_label[10];
    unsigned long user, nice, system, idle, iowait, irq, softirq, steal;

    // 2. Leemos los valores actuales
    // Formato: cpu  user nice system idle iowait irq softirq steal
    if (fscanf(f, "%s %lu %lu %lu %lu %lu %lu %lu %lu", cpu_label, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 9) {
        fprintf(stderr, "Error leyendo /proc/stat\n");
        fclose(f);
        return;
    }
    fclose(f);

    // 3. Calcular totales actuales y previos
    // Nota: Es común sumar iowait al idle, y (system + irq + softirq) al system.
    unsigned long cur_idle_all = idle + iowait;
    unsigned long cur_system_all = system + irq + softirq;
    unsigned long cur_user_all = user + nice;
    unsigned long cur_total = cur_idle_all + cur_system_all + cur_user_all + steal;

    unsigned long prev_idle_all = p_idle + p_iowait;
    unsigned long prev_system_all = p_system + p_irq + p_softirq;
    unsigned long prev_user_all = p_user + p_nice;
    unsigned long prev_total = prev_idle_all + prev_system_all + prev_user_all + p_steal;

    // 4. Calcular Deltas (Diferencia: Actual - Anterior)
    long total_delta = cur_total - prev_total;
    long idle_delta = cur_idle_all - prev_idle_all;
    long system_delta = cur_system_all - prev_system_all;
    long user_delta = cur_user_all - prev_user_all;

    // 5. Calcular porcentajes
    float cpu_usage_pct = 0.0, user_pct = 0.0, system_pct = 0.0, idle_pct = 0.0;

    // Evitamos división por cero (pasa en la primera ejecución o si es muy rápido)
    if (total_delta > 0) {
        // Fórmula del PDF: (Total - Idle) / Total * 100
        cpu_usage_pct = (float)(total_delta - idle_delta) / total_delta * 100.0;
        
        // Extra: porcentajes individuales para el reporte
        user_pct = (float)user_delta / total_delta * 100.0;
        system_pct = (float)system_delta / total_delta * 100.0;
        idle_pct = (float)idle_delta / total_delta * 100.0;
    }

    // 6. Actualizar las variables estáticas para la próxima vez
    p_user = user; p_nice = nice; p_system = system; p_idle = idle;
    p_iowait = iowait; p_irq = irq; p_softirq = softirq; p_steal = steal;

    // 7. Formatear salida según PDF
    // CPU;<ip_logica_agente>; <CPU_usage>; <user_pct>; <system_pct>; <idle_pct>
    snprintf(buffer, 256, "CPU;%s;%.2f;%.2f;%.2f;%.2f\n",
             ip_logica,
             cpu_usage_pct,
             user_pct,
             system_pct, 
             idle_pct);
}
    //--------------------------------------------------------------------------------------------------



    // Nos conectamos con el proceso viewer-------------------------------------------------------------
int connectToViewer(char *ip_recolector, int puerto){
    struct sockaddr_in client;
    

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("Error al crear socket");
        return -1;
    }

    client.sin_family = AF_INET;
    client.sin_port = htons(puerto);
    client.sin_addr.s_addr = inet_addr(ip_recolector);
    bzero(&client.sin_zero, 8);

    int r = connect(fd, (struct sockaddr *)&client, sizeof(struct sockaddr_in));
    if(r<0){
        perror("Error client connect");
        close(fd);
        return -1;
    }
    return fd;
}
    //--------------------------------------------------------------------------------------------------


// ./monitor    ip_recolector  puerto   ip_máquina
// argv[1] ip_recolector
// argv[2] puerto
// argv[3] ip_lógica_máquina\


// argc y el puntero argv[] son propios del SO
int main(int argc, char *argv[]) {

    if(argc != 4){
        fprintf(stderr, "Faltan argumentos");
        return 1;
    }

    char *ip_recolector = argv[1];
    int puerto = atoi(argv[2]); //atoi() convierte una string con un número a entero
    char *ip_logica = argv[3];

    char buffer[256];

    printf("Conectando a %s:%d...\n", ip_recolector, puerto);
    
    int socket_fd = connectToViewer(ip_recolector, puerto);
    if(socket_fd == -1) return -1;
    
    printf("Conectado. Iniciando envío de datos...\n");
    

    // envio cada 2 segundos de informacion
    // dejamos de enviar mensajes cuando el servidor rechaza conexion
    while(1){
        getMemInfo(buffer, ip_logica);
        if(send(socket_fd, buffer, strlen(buffer), 0) < 0){
            perror("Error enviando datos de memoria");
            break;
        }

        getCpuInfo(buffer, ip_logica);
        if(send(socket_fd, buffer, strlen(buffer), 0) < 0){
            perror("Error enviando datos de cpu");
            break;
        }
        sleep(2);
    }

    close(socket_fd);

    return 0;
}




