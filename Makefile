CC = gcc
CFLAGS = -Wall -Wextra -g

all: viewer monitor

viewer: viewer.c
	$(CC) $(CFLAGS) -o viewer viewer.c

monitor: monitor.c
	$(CC) $(CFLAGS) -o monitor monitor.c

clean:
	rm -f viewer monitor
