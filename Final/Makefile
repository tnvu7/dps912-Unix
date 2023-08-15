CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=processor.cpp
LIBS=-lpthread

processor: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o processor
	
all: processor