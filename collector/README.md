# Collector - Monitor Distribuido

Servidor central que recibe datos de CPU y memoria de múltiples agentes remotos.

## ⚠️ REQUISITOS

**Este código SOLO funciona en Linux** porque usa:
- Memoria compartida (System V IPC)
- Semáforos (System V IPC)
- Sockets POSIX

### Opciones para compilar:

#### ✅ **Linux nativo** (Ubuntu, Debian, Fedora, etc.)
```bash
make
```

#### ✅ **WSL (Windows Subsystem for Linux)**
```bash
# Si no tienes build-essential instalado:
sudo apt update && sudo apt install build-essential

# Compilar:
make
```

#### ✅ **macOS**
```bash
# Instalar herramientas de compilación:
xcode-select --install

# Compilar:
make
```

#### ❌ **Windows nativo (PowerShell/CMD)**
**No compatible.** Usa WSL o una VM Linux.

---

## 🚀 Compilación Rápida

```bash
make
```

Si no tienes `make`:
```bash
gcc -Wall -Wextra -pthread -std=c11 -g -c main.c
gcc -Wall -Wextra -pthread -std=c11 -g -c shared_data.c
gcc -Wall -Wextra -pthread -std=c11 -g -c network.c
gcc -pthread -o collector main.o shared_data.o network.o
```

---

## 🎯 Ejecución

```bash
./collector <puerto>
```

Ejemplo:
```bash
./collector 8080
```

---

## 🧹 Limpieza

```bash
# Limpiar archivos compilados
make clean

# Limpiar memoria compartida y semáforos
make clean-ipc
```

---

## 📡 Protocolo de Mensajes

### CPU
```
CPU;<ip>;<cpu_usage>;<user_pct>;<system_pct>;<idle_pct>\n
```

### MEM
```
MEM;<ip>;<mem_used_MB>;<mem_free_MB>;<swap_total_MB>;<swap_free_MB>\n
```

---

## 🔧 Solución de Problemas

### "make: command not found"
```bash
# Ubuntu/Debian:
sudo apt install build-essential

# Fedora/RHEL:
sudo dnf install gcc make

# macOS:
xcode-select --install
```

### "gcc: command not found"
```bash
# Instalar compilador
sudo apt install gcc
```

### Error al ejecutar en Windows
Este código **no funciona en Windows nativo**. Usa WSL:
```bash
# Instalar WSL (en PowerShell como administrador):
wsl --install

# Luego en WSL:
sudo apt update
sudo apt install build-essential
cd /mnt/c/ruta/a/collector
make
```

---

## 📊 Características

- ✅ Memoria compartida (0x1234)
- ✅ Semáforos (0x5678)
- ✅ Threads para múltiples conexiones
- ✅ Sockets TCP
- ✅ Hasta 4 hosts monitoreados

---

## 📝 Para el Equipo

**Si estás en Linux/WSL:** Solo ejecuta `make` y listo.

**Si estás en Windows:** Necesitas WSL. Instrucciones:
1. Abre PowerShell como administrador
2. Ejecuta: `wsl --install`
3. Reinicia
4. Abre Ubuntu desde el menú inicio
5. Ejecuta: `sudo apt update && sudo apt install build-essential`
6. Navega a la carpeta: `cd /mnt/c/ruta/al/proyecto/collector`
7. Compila: `make`
