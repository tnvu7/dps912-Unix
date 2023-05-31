CC = gcc
CFLAGS = -Wall -Wextra

TARGET = Lab1-ThuNgaVu

$(TARGET): Lab1-ThuNgaVu.o
	$(CC) $(CFLAGS) -o $(TARGET) Lab1-ThuNgaVu.o

Lab1-ThuNgaVu.o: Lab1-ThuNgaVu.c
	$(CC) $(CFLAGS) -c Lab1-ThuNgaVu.c

clean:
	rm -f $(TARGET) *.o
