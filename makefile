CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = 

PROG = backup

all: $(PROG)
debug: backup-debug

$(PROG): ConexaoRawSocket.o utils.o backup.o server.o client.o main.o
	$(CC) $(CFLAGS) -o $(PROG) ConexaoRawSocket.o utils.o backup.o server.o client.o main.o $(LFLAGS)

backup-debug: ConexaoRawSocket.o utils-debug.o backup-debug.o server-debug.o client-debug.o main-debug.o
	$(CC) $(CFLAGS) -DDEBUG -o $(PROG) ConexaoRawSocket.o utils-debug.o backup-debug.o server-debug.o client-debug.o main-debug.o $(LFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o $(LFLAGS)

main-debug.o: main.c
	$(CC) $(CFLAGS) -DDEBUG -c main.c -o main-debug.o $(LFLAGS)

client.o: client.h client.c
	$(CC) $(CFLAGS) -c client.c -o client.o $(LFLAGS)

client-debug.o: client.h client.c
	$(CC) $(CFLAGS) -DDEBUG -c client.c -o client-debug.o $(LFLAGS)

server.o: server.h server.c
	$(CC) $(CFLAGS) -c server.c -o server.o $(LFLAGS)

server-debug.o: server.h server.c
	$(CC) $(CFLAGS) -DDEBUG -c server.c -o server-debug.o $(LFLAGS)

backup.o: backup.h backup.c
	$(CC) $(CFLAGS) -c backup.c -o backup.o $(LFLAGS)

backup-debug.o: backup.h backup.c
	$(CC) $(CFLAGS) -DDEBUG -c backup.c -o backup-debug.o $(LFLAGS)

utils.o: utils.h utils.c
	$(CC) $(CFLAGS) -c utils.c -o utils.o $(LFLAGS)

utils-debug.o: utils.h utils.c
	$(CC) $(CFLAGS) -DDEBUG -c utils.c -o utils-debug.o $(LFLAGS)

ConexaoRawSocket.o: ConexaoRawSocket.h ConexaoRawSocket.c
	$(CC) $(CFLAGS) -c ConexaoRawSocket.c -o ConexaoRawSocket.o $(LFLAGS)

clean:
	rm -f *.o $(PROG)
