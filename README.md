# Sistema de Monitoreo Distribuido (Parcial 2 SO)

Este proyecto implementa un sistema de monitoreo distribuido cliente-servidor para visualizar el estado (CPU y Memoria) de múltiples máquinas remotas en tiempo real. Utiliza Sockets TCP para la comunicación a través de la red y Mecanismos de IPC (Memoria Compartida y Semáforos) para la gestión eficiente de datos en el servidor.

---

## 🏗️ Arquitectura del Sistema

El sistema se compone de tres partes principales:

1.  **Monitor (Agente/Cliente)**: Se ejecuta en cada máquina que queremos monitorear. Recolecta estadísticas del sistema y las envía al servidor.
2.  **Collector (Servidor Central)**: Recibe los datos de todos los monitores, los procesa y los almacena en memoria compartida.
3.  **Visualizador (Display)**: Muestra los datos almacenados en una tabla actualizada en tiempo real (integrado en el Collector).

---

## 📂 Desglose de Componentes

### 1. Monitor (`monitor.c`)
Este es el agente que recolecta la información.

*   **Función Principal**:
    *   Lee archivos del sistema `/proc` de Linux para obtener estadísticas reales.
    *   `/proc/meminfo`: Para datos de memoria RAM y Swap.
    *   `/proc/stat`: Para datos de uso de CPU (Usuario, Sistema, Idle).
*   **Comunicación**:
    *   Se conecta vía Socket TCP al `Collector`.
    *   Envía mensajes formateados cada 2 segundos.
    *   Formatos: `MEM;...` y `CPU;...`.

### 2. Collector (`collector/`)
El cerebro del sistema. Maneja múltiples conexiones y concurrencia.

#### `main.c` (Gestor Principal)
*   **Inicialización**: Configura la Memoria Compartida y los Semáforos (System V IPC) para guardar los datos de los hosts.
*   **Thread de Display**: Crea un hilo (`display_thread`) dedicado exclusivamente a limpiar la pantalla e imprimir la tabla de datos actualizada leyendo de la memoria compartida.
*   **Servidor TCP**: Escucha indefinidamente en un puerto. Por cada nuevo Monitor que se conecta, lanza un **nuevo hilo** (`handle_client`) para atenderlo sin bloquear a los demás.

#### `network.c` (Lógica de Red)
*   **`create_server_socket`**: Crea y configura el socket del servidor.
*   **`parse_message`**: Interpreta las cadenas de texto (`CPU;...`, `MEM;...`) que llegan de los monitores y extrae los valores numéricos.
*   **`handle_client`**: Es la función que ejecuta cada hilo de cliente. Recibe datos en bucle, los parsea y llama a `update_host_data`.
*   **`update_host_data`**: Escribe los datos en la memoria compartida. **Importante**: Usa un **semáforo** para bloquear el acceso mientras escribe, evitando condiciones de carrera (race conditions) si dos hilos intentan escribir al mismo tiempo.

#### `shared_data.c` (Gestión de Recursos)
*   Encapsula las llamadas al sistema operativo (`shmget`, `shmat`, `semget`, `semop`) para facilitar el uso de memoria compartida y semáforos, manteniendo el código principal limpio.

### 3. Viewer (`viewer.c`)
*   Este es un componente básico (posiblemente una versión antigua o de prueba) que actúa como un servidor simple que acepta una sola conexión, imprime un mensaje y se cierra.
*   **Nota**: La funcionalidad completa de visualización está actualmente integrada en el `Collector` (`display_thread` en `main.c`), por lo que este archivo no es el visualizador principal del proyecto final.

---

## 🔄 Flujo de Datos

1.  **Monitor** lee `/proc/stat` → Calcula % CPU.
2.  **Monitor** envía string: `CPU;192.168.1.10;15.5;0.5;5.0;94.5`
3.  **Collector** (Hilo Cliente) recibe string → Parsea valores.
4.  **Collector** solicita Semáforo (Lock).
5.  **Collector** actualiza estructura en Memoria Compartida.
6.  **Collector** libera Semáforo (Unlock).
7.  **Display Thread** lee Memoria Compartida → Imprime tabla en terminal.

---

## 🚀 Cómo Ejecutar

### Requisitos
*   Sistema Linux (o WSL en Windows).
*   Compilador `gcc` y herramienta `make`.

### 1. Compilar y Correr el Servidor (Collector)
En la terminal del servidor:
```bash
cd collector
make
./collector 8080
```
*(El servidor quedará esperando conexiones e imprimiendo la tabla)*

### 2. Compilar y Correr el Agente (Monitor)
En la terminal de la máquina a monitorear (o en otra terminal si es local):
```bash
# Compilar monitor.c (si no tienes Makefile en raíz):
gcc monitor.c -o monitor

# Ejecutar: ./monitor <IP_SERVIDOR> <PUERTO> <IP_LOGICA_ESTA_MAQUINA>
./monitor 127.0.0.1 8080 192.168.1.50
```
*   `IP_SERVIDOR`: IP donde corre el collector (usar `127.0.0.1` si es la misma máquina).
*   `PUERTO`: El mismo puerto que abrió el collector (ej. 8080).
*   `IP_LOGICA`: Un identificador (IP falsa o nombre) para mostrar en la tabla.

---
**Desarrollado para la asignatura Sistemas Operativos - Parcial 2**
