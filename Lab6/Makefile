CC=g++
CFLAGS=-I
CFLAGS+=-Wall
SERVER=server.cpp
CLIENT+=client.cpp

server: $(SERVER)
	$(CC) $(CFLAGS) -o server $(SERVER)

client: $(CLIENT)
	$(CC) $(CFLAGS) -o client $(CLIENT)

clean:
	rm -f *.o server client

all: server client