# Makefile para Collector
CC = gcc
CFLAGS = -Wall -Wextra -pthread -std=c11 -g
LDFLAGS = -pthread

OBJS = main.o shared_data.o network.o
TARGET = collector

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "✓ Collector compilado"

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "✓ Limpiado"

clean-ipc:
	@ipcs -m | grep 0x00001234 | awk '{print $$2}' | xargs -r ipcrm -m 2>/dev/null || true
	@ipcs -s | grep 0x00005678 | awk '{print $$2}' | xargs -r ipcrm -s 2>/dev/null || true
	@echo "✓ IPC limpiado"

run: $(TARGET)
	./$(TARGET) 8080

.PHONY: all clean clean-ipc run
