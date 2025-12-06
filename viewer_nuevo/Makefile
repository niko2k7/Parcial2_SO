# Makefile para Viewer
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS =

OBJS = main.o shared_data.o
TARGET = viewer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "✓ Viewer compilado"

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "✓ Limpiado"

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
