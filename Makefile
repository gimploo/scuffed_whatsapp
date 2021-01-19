CC= gcc
CCFLAGS= -Wall -Wextra -pedantic -g -pthread
OBJS= linkedlist.o common.o thread_pool.o
BIN= client server

all: $(BIN)
	rm *.o

client: tcp_client.c common.o
	$(CC) $(CCFLAGS) -o client tcp_client.c common.o

server: tcp_server.c $(OBJS)
	$(CC) $(CCFLAGS) -o server tcp_server.c $(OBJS)

linkedlist.o: linkedlist.c
	$(CC) $(CCFLAGS) -c linkedlist.c

common.o: common.c
	$(CC) $(CCFLAGS) -c common.c

thread_pool.o: thread_pool.c
	$(CC) $(CCFLAGS) -c thread_pool.c

clean: 
	rm $(BIN)
