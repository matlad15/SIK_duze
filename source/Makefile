CC = g++
FLAGS = -Wall -Wextra -O2 -std=c++17
EXEC_SERVER = screen-worms-server
EXEC_CLIENT = screen-worms-client
TARGET = screen-worms-client screen-worms-client.o screen-worms-server screen-worms-server.o

.PHONY: all

all: $(TARGET)


functions.o: functions.cpp
	$(CC) $(FLAGS) -c functions.cpp
	
screen-worms-server.o: screen-worms-server.cpp
	$(CC) $(FLAGS) -c screen-worms-server.cpp
	
screen-worms-client.o: screen-worms-client.cpp
	$(CC) $(FLAGS) -c screen-worms-client.cpp
	
screen-worms-client: screen-worms-client.o functions.o
	$(CC) screen-worms-client.o functions.o -o $(EXEC_CLIENT)
	
screen-worms-server: screen-worms-server.o
	$(CC) screen-worms-server.o -o $(EXEC_SERVER)

clean:
	rm *.o $(EXEC_CLIENT) $(EXEC_SERVER)
