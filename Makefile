CC= gcc
CCFLAGS= -Wall -Wextra -std=c11 -pedantic -g -pthread
OBJS= linkedlist.o
BIN= client server

all: $(BIN)

client: tcp_client.c  
	$(CC) $(CCFLAGS) -o client tcp_client.c

server: tcp_server.c linkedlist.o
	$(CC) $(CCFLAGS) -o server tcp_server.c linkedlist.o

linkedlist.o: linkedlist.c
	$(CC) -c linkedlist.c

clean: 
	rm $(BIN) $(OBJS) 
