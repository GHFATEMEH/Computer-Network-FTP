#include "functions.h"

using namespace std;

FileData getConfigFileData()
{
	Json::Value config;
	FileData f;
	ifstream configFile("config.json", ifstream::binary);
	configFile >> config;
	f.commandChannelPort = config["commandChannelPort"].asInt();
	f.dataChannelPort = config["dataChannelPort"].asInt();
	Json::Value& users = config["users"];
	for(int i = 0; i < users.size(); i++)
	{
		UserInfo newUser = UserInfo(users[i]["user"].asString(), users[i]["password"].asString(), 
										users[i]["admin"].asString() == "true" ? true : false, stoi(users[i]["size"].asString()));
		f.users.push_back(newUser);
	}
	Json::Value& files = config["files"];
	for(int i = 0; i < files.size(); i++)
		f.files.push_back(files[i].asString());
	return f;
}

vector<string> split(string line, char delimiter){
    stringstream stream(line);
    vector<string> outList;
    string block;
    while(getline(stream, block, delimiter)){
        outList.push_back(block);
    }
    return  outList;
}