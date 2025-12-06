# Viewer - Visualizador de Monitor Distribuido

Proceso que lee la memoria compartida del collector y muestra una tabla con las estadísticas de todos los hosts monitoreados.

## ⚠️ REQUISITOS

- **El collector debe estar ejecutándose** antes de iniciar el viewer
- Solo funciona en Linux/WSL/macOS (usa System V IPC)

## 🚀 Compilación

```bash
make
```

## 🎯 Ejecución

```bash
./viewer
```

**Importante:** Ejecuta primero el collector, luego el viewer.

## 📊 Formato de Tabla

```
╔═══════════════╦═══════╦═══════════╦══════════╦═══════════╦═════════════╦═════════════╗
║ IP            ║ CPU%  ║ CPU_user% ║ CPU_sys% ║ CPU_idle% ║ Mem_used_MB ║ Mem_free_MB ║
╠═══════════════╬═══════╬═══════════╬══════════╬═══════════╬═════════════╬═════════════╣
║ 192.168.0.2   ║  37.5 ║      15.0 ║      5.0 ║      57.5 ║     2048.50 ║     1024.25 ║
║ 192.168.0.3   ║  82.1 ║      60.0 ║     12.0 ║      27.9 ║     4096.00 ║      512.00 ║
╚═══════════════╩═══════╩═══════════╩══════════╩═══════════╩═════════════╩═════════════╝

Última actualización: Thu Dec  5 20:15:32 2025
```

## ⏱️ Características

- ✅ Actualización automática cada 2 segundos
- ✅ Limpia la pantalla en cada actualización (como pide el PDF)
- ✅ Muestra `--` para hosts sin datos recientes
- ✅ Timestamp de última actualización
- ✅ Usa semáforos para sincronización con collector

## 🔧 Solución de Problemas

### "Error: No se pudo acceder a la memoria compartida"
```bash
# El collector no está ejecutándose
# Solución: Ejecuta primero el collector
cd ../collector
./collector 8080
```

### "Error: No se pudo acceder al semáforo"
```bash
# El collector no está ejecutándose o terminó abruptamente
# Solución: Limpia recursos IPC y reinicia collector
cd ../collector
make clean-ipc
./collector 8080
```

## 📝 Uso Completo del Sistema

### Terminal 1: Collector
```bash
cd collector
make
./collector 8080
```

### Terminal 2: Viewer
```bash
cd viewer_nuevo
make
./viewer
```

### Terminal 3+: Monitors (agentes)
```bash
gcc -o monitor monitor.c
./monitor 127.0.0.1 8080 192.168.0.2
```

## 🛑 Detener el Viewer

Presiona `Ctrl+C` para salir.

---

## 📋 Cumple Requisitos del PDF

✅ Lee la estructura compartida (memoria compartida con collector)
✅ Limpia la pantalla (`printf("\033[2J\033[H")`)
✅ Muestra tabla cada cierto intervalo (2 segundos)
✅ Muestra `--` si no hay datos recientes
✅ Usa semáforos para sincronizar con collector
