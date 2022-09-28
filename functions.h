#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <json/value.h>
#include <json/json.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

class UserInfo{
    public:
        UserInfo(std::string user, std::string password, bool admin, int size):user(user), password(password), admin(admin), size(size) {}
        std::string user;
        std::string password;
        bool admin;
        int size;
};

struct FileData
{
	int commandChannelPort;
	int dataChannelPort;
	std::vector<UserInfo> users;
	std::vector<std::string> files;
};

std::vector<std::string> split(std::string line, char delimiter);
FileData getConfigFileData();

#endif