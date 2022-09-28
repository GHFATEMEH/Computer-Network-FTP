#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include "dirent.h"
#include <pwd.h>
#include <sys/types.h>
#include <sstream>
#include "functions.h"

#define MAX_CLIENTS 100

class Server{
    public:
        Server(FileData f);
        void listenOnSocket();
        void repeatAction();

    private:
        int serverFdCommand;
        int serverFdData;
        std::map<int, std::string> connectedClients;
        struct sockaddr_in addrCommand;
        struct sockaddr_in addrData;
        std::vector<UserInfo> users;
        std::vector<std::string> files;
        void buildDataSocket(int dataPort);
        void buildCommandSocket(int commandPort);
        void detectCommand(int commandChannel, std::string clientInput);
        void createLogFile();
        void writeInLogFile(std::string username, std::string dataToWrite);
        void userCommand(int commandChannel, std::string input);
        void passCommand(int commandChannel, std::string input);
        void pwdCommand(int commandChannel, std::string clientInput);
        void mkdCommand(int commandChannel, std::string input);
        void deleCommand(std::string input, int commandChannel);
        void deleFileCommand(int commandChannel, std::string fileName);
        void deleDirectoryPathCommmand(int commanChannel, std::string directoryPath);
        void lsCommand(int commandChannel, std::string input);
        void cwdCommand(int commandChannel, std::string path);
        void renameCommand(int commandChannel, std::string input);
        void retr(int commandChannel, std::string input);
        void helpCommand(int commandChannel, std::string input);
        void quitCommand(int commandChannel, std::string input);
        void sendError332(int commandChannel);
        void sendError501(int commandChannel);
        void sendError500(int commandChannel);
        void sendError425(int commandChannel);
        void sendError503(int commandChannel);
        void sendError430(int commandChannel);
        void sendError550(int commandChannel);
        bool checkUserName(std::string user);
        bool checkUserLoggedIn(int commandChannel);
        bool checkIfAdminFile(std::string fileName);
        bool checkUserAccess(int commandChannel, std::string fileName);
        bool checkPass(std::vector<std::string> data);
        int getFileSize(std::string fileName);
        bool checkDownloadSizeOfUser(int fileSize, int commandChannel);
        std::vector<std::string> readFile(std::ifstream& file);
        std::string convertVectorToString(std::vector<std::string> v);
};

#endif