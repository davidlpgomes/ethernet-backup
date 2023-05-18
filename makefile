CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = 

PROG = backup

all: $(PROG)

$(PROG): ConexaoRawSocket.o utils.o backup.o server.o client.o main.o
	$(CC) $(CFLAGS) -o $(PROG) ConexaoRawSocket.o utils.o backup.o server.o client.o main.o $(LFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o $(LFLAGS)

client.o: client.h client.c
	$(CC) $(CFLAGS) -c client.c -o client.o $(LFLAGS)

server.o: server.h server.c
	$(CC) $(CFLAGS) -c server.c -o server.o $(LFLAGS)

backup.o: backup.h backup.c
	$(CC) $(CFLAGS) -c backup.c -o backup.o $(LFLAGS)

utils.o: utils.h utils.c
	$(CC) $(CFLAGS) -c utils.c -o utils.o $(LFLAGS)

ConexaoRawSocket.o: ConexaoRawSocket.h ConexaoRawSocket.c
	$(CC) $(CFLAGS) -c ConexaoRawSocket.c -o ConexaoRawSocket.o $(LFLAGS)

clean:
	rm -f *.o $(PROG)
