# Collector - Monitor Distribuido

Servidor central con **balance entre modularidad de archivos y funciones**.

## 📁 Estructura Balanceada (7 archivos)

```
collector/
├── config.h          # Constantes de configuración
├── shared_data.h     # Estructuras y funciones IPC
├── shared_data.c     # Implementación de memoria compartida y semáforos
├── network.h         # Funciones de red y procesamiento
├── network.c         # Sockets, parseo, almacenamiento, clientes
├── main.c            # Programa principal
└── Makefile          # Compilación
```

## 🎯 Organización Lógica

### 1. **config.h** - Configuración
- Constantes del sistema
- Claves IPC
- Tamaños de buffers

### 2. **shared_data.h/.c** - IPC (System V)
- Estructuras de datos compartidos
- Funciones de memoria compartida
- Funciones de semáforos

### 3. **network.h/.c** - Red y Procesamiento
- Creación de socket servidor
- Parseo de mensajes CPU/MEM
- Almacenamiento en memoria compartida
- Manejo de threads de clientes

### 4. **main.c** - Programa Principal
- Inicialización de recursos
- Loop de aceptación de conexiones
- Limpieza de recursos

## 🔧 Compilación

```bash
make
```

## 🚀 Ejecución

```bash
./collector 8080
```

## ✨ Balance Logrado

✅ **No demasiados archivos** - Solo 7 archivos (vs 16 original)
✅ **No todo en uno** - Separación lógica por responsabilidad
✅ **Fácil de navegar** - Cada archivo tiene un propósito claro
✅ **Modularidad por funciones** - Funciones bien organizadas dentro de cada archivo
