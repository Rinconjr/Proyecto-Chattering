CC = gcc
CFLAGS = -Wall -Wextra

all: manager talker

manager: manager.o
	$(CC) $(CFLAGS) -o manager manager.o

talker: talker.o
	$(CC) $(CFLAGS) -o talker talker.o

manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c

talker.o: talker.c
	$(CC) $(CFLAGS) -c talker.c

clean:
	rm -f manager talker *.o