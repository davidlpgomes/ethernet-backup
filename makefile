CC = gcc
CFLAGS = -Wall
LFLAGS = 

PROG = backup

all: $(PROG)

$(PROG): ConexaoRawSocket.o main.o
	$(CC) $(CFLAGS) -o $(PROG) ConexaoRawSocket.o main.o $(LFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o $(LFLAGS)

ConexaoRawSocket.o: ConexaoRawSocket.h ConexaoRawSocket.c
	$(CC) $(CFLAGS) -c ConexaoRawSocket.c -o ConexaoRawSocket.o $(LFLAGS)

clean:
	rm -f *.o $(PROG)
