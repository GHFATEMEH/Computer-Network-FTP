CC := g++ -std=c++11

all: Client.out Server.out

Server.out: Server.o functions.o
	$(CC) -o Server.out Server.o functions.o -ljsoncpp

Client.out: Client.o functions.o
	$(CC) -o Client.out Client.o functions.o -ljsoncpp

functions.o: functions.cpp functions.h
	$(CC) -c functions.cpp

Server.o: Server.cpp Server.h functions.h
	$(CC) -c Server.cpp

Client.o: Client.cpp Client.h functions.h
	$(CC) -c Client.cpp

.PHONY: clean
clean:
	rm *.o
	rm Client.out
	rm Server.out