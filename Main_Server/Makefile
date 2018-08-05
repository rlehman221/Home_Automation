CC = g++
CFLAGS  = -g -Wall -std=c++11 -pthread

TARGET = server
OTHER = client


all: $(TARGET) 

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)
