CC = g++

TARGET = Lab2

$(TARGET): Lab2.o
	$(CC) -o $(TARGET) Lab2.o libPidUtil.a

Lab2.o: Lab2.cpp
	$(CC) -c Lab2.cpp

clean:
	rm -f $(TARGET) *.o