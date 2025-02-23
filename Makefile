CC=g++

CFLAGS=-Wall -W -g -Werror 

bad: bclient bserver

bclient: ./bad_code/client.c
	$(CC) ./bad_code/client.c $(CFLAGS) -o badclient

bserver: ./bad_code/server.c 
	$(CC) ./bad_code/server.c $(CFLAGS) -o badserver

good: client server

client: client.c
	$(CC) client.c $(CFLAGS) -o client

server: server.c
	$(CC) server.c $(CFLAGS) -o server

clean:
	rm -f client server badserver badclient *.o

