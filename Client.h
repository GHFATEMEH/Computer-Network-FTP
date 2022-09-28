#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "functions.h"

#define COMMAND_DELIM ' '

class Client
{
public:
	Client(int commandPort, int dataPort);
	void run_client();
private:
	int commandSockServ;
	int dataSockServ;
	int myDataFd;
	std::string currUser;
	void buildCommandSocket(int commandPort);
    void buildDataSocket(int dataPort);
};

#endif