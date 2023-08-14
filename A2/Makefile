CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=Logger.cpp Automobile.cpp TravelSimulator.cpp
FILES2=LogServer.cpp
LIBS=-lpthread

travel: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

logserver: $(FILES2)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o travel logserver
	
all: travel logserver
