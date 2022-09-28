#include "Client.h"

using namespace std;

void Client::run_client()
{
	string input;
	int valread;
    char dataRecv[5000];
    char data[10000];
    while(getline(cin, input))
    {
        vector<string> splitedCommand = split(input, COMMAND_DELIM);
        if(splitedCommand[0] == "ls" || splitedCommand[0] == "retr")
            input += ' ' + to_string(myDataFd);
        if(splitedCommand[0] == "pass")
            input += ' ' + currUser;
        int n = input.length();
        char charArray[n + 1];
        strcpy(charArray, input.c_str());
        send(commandSockServ, charArray, strlen(charArray), 0);
	    if(splitedCommand[0] == "ls" || splitedCommand[0] == "retr")
	    {
            memset(data, 0, sizeof(data));
            valread = read(dataSockServ, data, 10000);
            data[valread] = '\0';
            string d(data);
            cout << d;
	    }
        if(splitedCommand[0] == "quit")
            currUser = "*";

        memset(dataRecv, 0, sizeof(dataRecv));
        valread = read(commandSockServ, dataRecv, 5000);
        dataRecv[valread] = '\0';
        string s(dataRecv);
        cout << s;
        if(splitedCommand[0] == "user"){
            if(s != "430: Invalid username or password")
                currUser = splitedCommand[1];
            if(currUser == "")
                currUser = "*";
        }

    }
    close(commandSockServ);
    close(dataSockServ);
}

Client::Client(int commandPort, int dataPort)
{
    currUser = "*";
    buildCommandSocket(commandPort);
    buildDataSocket(dataPort);
}

void Client::buildCommandSocket(int commandPort)
{
    struct sockaddr_in servAddrCommand;
    if ((commandSockServ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "\n Socket creation error \n";
        exit(EXIT_FAILURE);
    }
    servAddrCommand.sin_family = AF_INET;
    servAddrCommand.sin_port = htons(8000);
    if(inet_pton(AF_INET, "127.0.0.1", &servAddrCommand.sin_addr)<=0)
    {
        cerr << "\nInvalid address/ Address not supported \n";
        exit(EXIT_FAILURE);
    }

    if(connect(commandSockServ, (struct sockaddr *)&servAddrCommand, sizeof(servAddrCommand)) < 0)
    {
        cerr << "\nConnection Failed \n";
        exit(EXIT_FAILURE);
    }
}

void Client::buildDataSocket(int dataPort)
{
    char buffer[1024] = {0};
    struct sockaddr_in servAddrData;
    if ((dataSockServ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "\n Socket creation error \n";
        exit(EXIT_FAILURE);
    }
    servAddrData.sin_family = AF_INET;
    servAddrData.sin_port = htons(8001);
    if(inet_pton(AF_INET, "127.0.0.1", &servAddrData.sin_addr)<=0)
    {
        cerr << "\nInvalid address/ Address not supported \n";
        exit(EXIT_FAILURE);
    }

    if(connect(dataSockServ, (struct sockaddr *)&servAddrData, sizeof(servAddrData)) < 0)
    {
        cerr << "\nConnection Failed \n";
        exit(EXIT_FAILURE); 
    }
    read(dataSockServ, buffer, 1024);
    string s(buffer);
    myDataFd = stoi(s);
}

int main()
{
    FileData f = getConfigFileData();
    Client client = Client(f.commandChannelPort, f.dataChannelPort);
    client.run_client();
}