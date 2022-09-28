#include "Server.h"

using namespace std;

vector<string> splitInput(string input){
    vector<string>tokenOfInput;
    istringstream ss(input);
    string word;
    while(ss >> word)
        tokenOfInput.push_back(word);
    return tokenOfInput;
}

Server::Server(FileData f){
    users = f.users;
    files = f.files;
    createLogFile();
    buildCommandSocket(f.commandChannelPort);
    buildDataSocket(f.dataChannelPort);
}

void Server::buildCommandSocket(int commandPort){
    serverFdCommand = socket(AF_INET, SOCK_STREAM, 0);

    if(serverFdCommand == 0){
        cerr << "Building socket for command failed\n";
        exit(EXIT_FAILURE);
    }

    
    addrCommand.sin_family = AF_INET;
    addrCommand.sin_port = htons(commandPort);
    addrCommand.sin_addr.s_addr = inet_addr("127.0.0.1");;
    
    if(::bind(serverFdCommand, (struct sockaddr* )& addrCommand, sizeof(addrCommand)) < 0){
        cerr << "Bind for command channel failed.\n";
        exit(EXIT_FAILURE);
    }
}

void Server::buildDataSocket(int dataPort){
    serverFdData = socket(AF_INET, SOCK_STREAM, 0);

    if(serverFdData == 0){
        cerr << "Building socket for data failed\n";
        exit(EXIT_FAILURE);
    }

    addrData.sin_family = AF_INET;
    addrData.sin_port = htons(dataPort);
    addrData.sin_addr.s_addr = inet_addr("127.0.0.1");;
    
    if(::bind(serverFdData, (struct sockaddr*)& addrData, sizeof(addrData)) < 0){
        cerr << "Bind for data channel failed.\n";
        exit(EXIT_FAILURE);
    }
}

void Server::listenOnSocket(){
    if(listen(serverFdCommand, SOMAXCONN) < 0){
        cerr << "Error in listening on port command.\n";
        exit(EXIT_FAILURE);
    }

    if(listen(serverFdData, SOMAXCONN) < 0){
        cerr << "Error in listening on port data.\n";
        exit(EXIT_FAILURE);
    }
}

void Server::repeatAction(){
    int maxFd, newSocketCommand, newSocketData, receiveValue;
    int clientFd[MAX_CLIENTS] = {0};
    fd_set readfd;
    char commandData[5000];
    while(1){
        FD_ZERO(&readfd);
        FD_SET(serverFdCommand, &readfd);
        FD_SET(serverFdData, &readfd);
        maxFd = max(serverFdData, serverFdCommand);


        for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(clientFd[i] > 0)
				FD_SET(clientFd[i] , &readfd);
			if(clientFd[i] > maxFd)
				maxFd = clientFd[i];
		}
        
        
        select(maxFd + 1, &readfd, NULL, NULL, NULL);


        if(FD_ISSET(serverFdCommand, &readfd)){
            int addrCommandSize = sizeof(addrCommand);
			newSocketCommand = accept(serverFdCommand, (struct sockaddr *)&addrCommand, (socklen_t*)&addrCommandSize);
			for(int i = 0; i < MAX_CLIENTS; i++){
				if(clientFd[i] == 0){
					clientFd[i] = newSocketCommand;
					break;
				}
			}
		}


        if(FD_ISSET(serverFdData, &readfd)){
            int addrDataSize = sizeof(addrData);
			newSocketData = accept(serverFdData, (struct sockaddr *)&addrData, (socklen_t*)&addrDataSize);
			int n = to_string(newSocketData).length();
            char charArray[n + 1];
            strcpy(charArray, to_string(newSocketData).c_str());
            send(newSocketData, charArray, strlen(charArray), 0);
		}

        

        for(int i = 0; i < MAX_CLIENTS; i++){
            if(FD_ISSET(clientFd[i], &readfd)){
                receiveValue = recv(clientFd[i], commandData, 5000, 0);
                commandData[receiveValue] = '\0';
                if (receiveValue < 0){
                    close(clientFd[i]);
                    clientFd[i] = 0;
                    cerr << "A client got disconnected.\n";
                }
                else{
                    string clientInput(commandData);
                    detectCommand(clientFd[i], clientInput);
                    memset(commandData, 0, sizeof(commandData));
                }
            }
        }

    }

}

void Server::detectCommand(int commandChannel, string clientInput){
    string firstCommandPart = clientInput.substr(0, clientInput.find(' '));
    if(firstCommandPart == "user")
        userCommand(commandChannel, clientInput);
    else if(firstCommandPart == "pass")
        passCommand(commandChannel, clientInput);
    else if(firstCommandPart == "pwd")
        pwdCommand(commandChannel, clientInput);
    else if(firstCommandPart == "mkd")
        mkdCommand(commandChannel, clientInput);
    else if(firstCommandPart == "dele")
        deleCommand(clientInput, commandChannel);
    else if(firstCommandPart == "ls")
        lsCommand(commandChannel, clientInput);
    else if(firstCommandPart == "cwd")
        cwdCommand(commandChannel, clientInput);
    else if(firstCommandPart == "rename")
        renameCommand(commandChannel, clientInput);
    else if(firstCommandPart == "retr")
        retr(commandChannel, clientInput);
    else if(firstCommandPart == "help")
        helpCommand(commandChannel, clientInput);
    else if(firstCommandPart == "quit")
        quitCommand(commandChannel, clientInput);
    else
        sendError501(commandChannel);
}

/*-------------------------------------------------------------------------------------*/

void Server::createLogFile(){
    ifstream logFile;
    logFile.open("log.txt");
    if(!logFile){
        ofstream log("log.txt");
        log.close();
    }
}

/*-------------------------------------------------------------------------------------*/

void Server::writeInLogFile(string username, string dataToWrite){
    time_t systemTime = time(NULL);
    ofstream file;
    file.open("log.txt", ios::app);
    file << string(ctime(&systemTime)) + "Username:" + username + "/" + dataToWrite + "\n";

}

/*-------------------------------------------------------------------------------------*/

bool Server::checkUserLoggedIn(int commandChannel)
{
    if(connectedClients.find(commandChannel) == connectedClients.end())
      return false;
    if(connectedClients[commandChannel] == "*")
        return false;
    return true;
}

/*-------------------------------------------------------------------------------------*/
bool Server::checkUserName(string user)
{
    for(int i = 0; i < users.size(); i++)
        if(users[i].user == user)
            return true;
    return false;
}

void Server::userCommand(int commandChannel, string input){
    vector<string>data = splitInput(input);
    if(data.size() != 2){
        sendError501(commandChannel);
        return;
    }
    if(!checkUserName(data[1]))
    {
        sendError430(commandChannel);
        return;
    }
    char answer[200] = "331: User name okay, need password.\n";
    send(commandChannel, answer, strlen(answer), 0);
}

/*-------------------------------------------------------------------------------------*/

bool Server::checkPass(vector<string> data)
{
    for(int i = 0; i < users.size(); i++)
        if(users[i].user == data[2] && users[i].password == data[1])
            return true;
    return false;
}

void Server::passCommand(int commandChannel, string input){
    vector<string>data = splitInput(input);
    if(data.size() != 3){
        sendError501(commandChannel);
        return;
    }
    if(data[2] == "*"){
        sendError503(commandChannel);
        return;
    }
    if(!checkPass(data)){
        sendError430(commandChannel);
        return;
    }
    char answer[200] = "230: User logged in, proceed. Logged out if appropriate.\n";
    send(commandChannel, answer, strlen(answer), 0);
    connectedClients[commandChannel] = data[2];
    writeInLogFile(data[2], "User " + data[2]  + " logged in successfully.");
}

/*-------------------------------------------------------------------------------------*/
void Server::pwdCommand(int commandChannel, string clientInput){
    vector<string>data = splitInput(clientInput);
    if(data.size() != 1){
        sendError501(commandChannel);
        return;
    }
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    char temp[256];
    getcwd(temp, 256);
    string currentDirectory = string(temp);
    string s = "257: " + currentDirectory + "\n";
    int n = s.length();
    char answer[n + 1];
    strcpy(answer, s.c_str());
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(connectedClients[commandChannel], "User got current directory.");
}

/*-------------------------------------------------------------------------------------*/

void Server::mkdCommand(int commandChannel, string input){
    vector<string>data = splitInput(input);
    if(data.size() != 2){
        sendError501(commandChannel);
        return;
    }
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    if(mkdir(data[1].c_str(), 0777) != 0){
        sendError500(commandChannel);
        return;
    }
    string s = "257: " + data[1] + " created.\n";
    int n = s.length();
    char answer[n + 1];
    strcpy(answer, s.c_str());
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(connectedClients[commandChannel], "User made directory at " + data[1] + ".");
}

/*-------------------------------------------------------------------------------------*/

bool Server::checkIfAdminFile(string fileName){
    for(int i = 0; i < files.size(); i++)
        if(files[i] == fileName)
            return true;
    return false;
}

bool Server::checkUserAccess(int commandChannel, string fileName){
    string userName = connectedClients[commandChannel];
    for(int i = 0; i < users.size(); i++){
        if(users[i].user == userName){
            if(users[i].admin)
                return true;
            if(checkIfAdminFile(fileName))
                return false;
            return true;
        }
    }
    return true;
}

void Server::deleCommand(string input, int commandChannel){
    vector<string>data = splitInput(input);
    if(data.size() != 3){
        sendError501(commandChannel);
        return;
    }
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    if(data[1] == "-f"){
        if(!checkUserAccess(commandChannel, data[2])){
            sendError550(commandChannel);
            return;
        }
        deleFileCommand(commandChannel, data[2]);
        writeInLogFile(connectedClients[commandChannel], "User deleted file " + data[2] + ".");
    }
    else if(data[1] == "-d"){
        deleDirectoryPathCommmand(commandChannel, data[2]);
        writeInLogFile(connectedClients[commandChannel], "User deleted directory " + data[2] + ".");
    }
    else
        sendError501(commandChannel);
}

/*-------------------------------------------------------------------------------------*/

void Server::deleFileCommand(int commandChannel, string fileName){
    if(remove(fileName.c_str()) != 0){
        sendError500(commandChannel);
        return;
    }
    string s = "250: " + fileName + " deleted.\n";
    int n = s.length();
    char answer[n + 1];
    strcpy(answer, s.c_str());
    send(commandChannel, answer, strlen(answer), 0);

}

/*-------------------------------------------------------------------------------------*/

//does not work
void Server::deleDirectoryPathCommmand(int commandChannel, string directoryPath){
    if(rmdir(directoryPath.c_str()) != 0){
        sendError500(commandChannel);
        return;
    }
    string s = "250: " + directoryPath + " deleted.\n";
    int n = s.length();
    char answer[n + 1];
    strcpy(answer, s.c_str());
    send(commandChannel, answer, strlen(answer), 0);
}

/*-------------------------------------------------------------------------------------*/

string Server::convertVectorToString(vector<string> v)
{
    string s = "";
    for(int i = 0; i < v.size(); i++)
        s += v[i] + "\n";
    return s;
}

void Server::lsCommand(int commandChannel, string input){
    vector<string>data = splitInput(input);
    for (int i = 0; i < data.size(); i++)
    if(data.size() != 2){
        send(stoi(data[data.size() - 1]), "\n", strlen("\n") , 0);
        sendError501(commandChannel);
        return;
    }
    if(!checkUserLoggedIn(commandChannel)){
        send(stoi(data[1]), "\n", strlen("\n") , 0);
        sendError332(commandChannel);
        return;
    }
    vector<string>lsResult;
    char answer[200] = "226: List transfer done.\n";
    char temp[256];
    getcwd(temp, 256);
    string currentDirectory = string(temp);
    struct dirent *entry;
    DIR *dir = opendir(currentDirectory.c_str());
    while((entry = readdir(dir)) != NULL )
        lsResult.push_back(entry->d_name);
    closedir(dir);
    string s = convertVectorToString(lsResult);
    int n = s.length();
    char charArray[n + 1];
    strcpy(charArray, s.c_str());
    send(stoi(data[1]), charArray, strlen(charArray) , 0);
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(connectedClients[commandChannel], "User got list of file in current directory.");
}

/*-------------------------------------------------------------------------------------*/

void Server::cwdCommand(int commandChannel, string path){
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    vector<string> data = splitInput(path);
    const char* directory;
    string currentDirectory;
    char answer[200] = "250: Successful change.\n";
    if (path == "cwd"){
        struct passwd *pass = getpwuid(getuid());
        directory = pass->pw_dir;
    }
    else if(data.size() == 2)
        directory = data[1].c_str();
    else{
        sendError501(commandChannel);
        return;
    }

    if (chdir(directory) < 0){
        sendError500(commandChannel);
        return;
    }
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(connectedClients[commandChannel], "User changed the directory.");
}

/*-------------------------------------------------------------------------------------*/

void Server::renameCommand(int commandChannel, string input){
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    char answer[200] = "250: Successful change.\n";
    vector<string>data = splitInput(input);
    if(data.size() != 3){
        sendError501(commandChannel);
        return;
    }
    if(!checkUserAccess(commandChannel, data[1])){
        sendError550(commandChannel);
        return;
    }
    if(rename(data[1].c_str(), data[2].c_str())!= 0){
        sendError500(commandChannel);
        return;
    }
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(connectedClients[commandChannel], "User changed file " + data[1] + " to " + data[2] + ".");
}

/*-------------------------------------------------------------------------------------*/

void Server::retr(int commandChannel, string input){
    vector<string>data = splitInput(input);
    char answer[200] = "Successful Download.\n";
    vector<string>fileDatas;
    if(data.size() != 3){
        send(stoi(data[data.size() - 1]), "\n", strlen("\n") , 0);
        sendError501(commandChannel);
        return;
    }
    if(!checkUserAccess(commandChannel, data[1])){
        send(stoi(data[2]), "\n", strlen("\n") , 0);
        sendError550(commandChannel);
        return;
    }
    ifstream file(data[1]);
    if(!file.is_open()){
        send(stoi(data[2]), "\n", strlen("\n") , 0);
        sendError500(commandChannel);
        return;
    }
    int fileSize = getFileSize(data[1]) / 1024;
    if(!checkDownloadSizeOfUser(fileSize, commandChannel)){
        send(stoi(data[2]), "\n", strlen("\n") , 0);
        sendError425(commandChannel);
        return;
    }
    fileDatas = readFile(file);
    string s = convertVectorToString(fileDatas);
    int n = s.length();
    char charArray[n + 1];
    strcpy(charArray, s.c_str());
    send(stoi(data[2]), charArray, strlen(charArray) , 0);
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(connectedClients[commandChannel], "User download file " + data[1] + "\n");
}

int Server::getFileSize(string fileName){
    ifstream file(fileName, ios::binary);
    file.seekg(0, ios::end);
    return file.tellg();
}

bool Server::checkDownloadSizeOfUser(int fileSize, int commandChannel){
    string userName = connectedClients[commandChannel];
    for(int i = 0; i < users.size(); i++){
        if(users[i].user == userName){
            if(users[i].size >= fileSize){
                users[i].size = users[i].size - fileSize;
                return true;
            }
        }
    }
    return false;
}

vector<string> Server::readFile(ifstream& file){
    string line;
    vector<string>data;
    while(getline(file, line))
        data.push_back(line);
    file.close();
    return data;
}

/*-------------------------------------------------------------------------------------*/

void Server::helpCommand(int commandChannel, string input){
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    vector<string>data = splitInput(input);
    if(data.size() != 1){
        sendError501(commandChannel);
        return;
    }
    string answer;
    answer = "214\n";
    answer += "user [username], Its argument is used to specify the user's string.It is used for user authentication.\n";
    answer += "pass [password], Its argument is used to check password is correct.\n";
    answer += "pwd, pwd print the current working directory.\n";
    answer += "mkd [directory path], mkd create new directory in directory path with name directory path.\n";
    answer += "dele -f [file name], dele -f delete file.Its argument is used to specify name of file.\n";
    answer += "dele -d [directory path], dele -d delete directory.Its argument is used to specify path of directory.\n";
    answer += "ls, ls list name of all files in the current working directory.\n";
    answer += "cwd <path>, cwd change the directory.Its argument specify path of directory.If path is empty it change to home directory.cwd.. go to previous directory.\n";
    answer += "rename [from] [to], It rename file in directory.[from] input shows old name of file and [to] shows new name.\n ";
    answer += "retr [name], retr download file.Its argument specify name of file.\n";
    answer += "help, It shows information about using commands.\n";
    answer += "quit, quit logout user from server.\n";
    int n = answer.length();
    char charArray[n + 1];
    strcpy(charArray, answer.c_str());
    send(commandChannel, charArray, strlen(charArray), 0);
}

/*-------------------------------------------------------------------------------------*/

void Server::quitCommand(int commandChannel, string input){
    if(!checkUserLoggedIn(commandChannel)){
        sendError332(commandChannel);
        return;
    }
    vector<string>data = splitInput(input);
    if(data.size() != 1){
        sendError501(commandChannel);
        return;
    }
    string user = connectedClients[commandChannel];
    connectedClients[commandChannel] = "*";
    char answer[200] = "221: Successful Quit.\n";
    send(commandChannel, answer, strlen(answer), 0);
    writeInLogFile(user, "User " + user + " logout from system.\n");
}

/*-------------------------------------------------------------------------------------*/

void Server::sendError332(int commandChannel){
    char answer[200] = "332: Need account for login.\n";
    send(commandChannel, answer, strlen(answer), 0);
}

/*-------------------------------------------------------------------------------------*/

void Server::sendError501(int commandChannel){
    char answer[200] = "501: Syntax error in parameters or arguments.\n";
    send(commandChannel, answer, strlen(answer), 0);
}

/*-------------------------------------------------------------------------------------*/

void Server::sendError500(int commandChannel){
    char answer[200] = "500: Error\n";
    send(commandChannel, answer, strlen(answer), 0);
}

/*-------------------------------------------------------------------------------------*/

void Server::sendError425(int commandChannel){
    char answer[200] = "425: Can't open data connection.\n";
    send(commandChannel, answer, strlen(answer), 0);
}

void Server::sendError503(int commandChannel){
    char answer[200] = "503: Bad sequence of commands.\n";
    send(commandChannel, answer, strlen(answer), 0);
}

void Server::sendError430(int commandChannel){
    char answer[200] = "430: Invalid username or password\n";
    send(commandChannel, answer, strlen(answer), 0);
}

void Server::sendError550(int commandChannel){
    char answer[200] = "550: File unavailable.\n";
    send(commandChannel, answer, strlen(answer), 0);
}

int main()
{
    FileData f = getConfigFileData();
    Server server = Server(f);
    server.listenOnSocket();
    server.repeatAction();
}