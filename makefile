CC = gcc
CFLAGS = -Wall
LFLAGS = 

PROG = backup

all: $(PROG)

$(PROG): ConexaoRawSocket.o utils.o main.o
	$(CC) $(CFLAGS) -o $(PROG) ConexaoRawSocket.o utils.o main.o $(LFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o $(LFLAGS)

utils.o: utils.h utils.c
	$(CC) $(CFLAGS) -c utils.c -o utils.o $(LFLAGS)

ConexaoRawSocket.o: ConexaoRawSocket.h ConexaoRawSocket.c
	$(CC) $(CFLAGS) -c ConexaoRawSocket.c -o ConexaoRawSocket.o $(LFLAGS)

clean:
	rm -f *.o $(PROG)
