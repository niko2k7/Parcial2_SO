#ifndef VIEWER_CONFIG_H
#define VIEWER_CONFIG_H

// Configuración del viewer
#define MAX_HOSTS 4
#define IP_LENGTH 32
#define REFRESH_INTERVAL 2  // Segundos entre actualizaciones

// Claves IPC (deben coincidir con el collector)
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// Timeout para marcar host como inactivo (segundos)
#define INACTIVE_TIMEOUT 10

#endif // VIEWER_CONFIG_H
