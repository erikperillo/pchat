#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include "hash.h"
#include "net.h"
#include "protocol.h"

using namespace std;

//number of digits to display from message hash
#define HASH_CUT 6
//maximum number of characters for file path
#define MAX_PATH_SIZE 2048
//commands available to user
#define REGISTER_CMD "register" 
#define EXIT_CMD "exit"
#define CREATE_GROUP_CMD "createg"
#define JOIN_GROUP_CMD "joing"
#define SEND_GROUP_CMD "sendg"
#define SEND_MSG_CMD "send"
#define SEND_FILE_CMD "sendf"
#define WHO_CMD "who"
#define ACCEPT_FILE_CMD "accept"
#define HELP_CMD "help"

mutex files_mtx;
map <string, string> files;

/*
Displays error message and exits.
*/
void error(const string& msg, int ret=1)
{
	perror(msg.c_str());
	exit(ret);
}

/*
Shows some info message with prompt in front of it.
*/
void info(const string& msg, const string& prompt="[chat] ")
{
	cout << prompt << msg << endl;
}

/*
Gets the path of directory where program is running. UNIX SYSTEMS ONLY.
*/
string pwd()
{
  char path[MAX_PATH_SIZE];
  ssize_t size = readlink("/proc/self/exe", path, MAX_PATH_SIZE);

  return string(path, (size > 0)?size:0);
}

/*
Displays a help message containing all possible commands to be used.
*/
void displayHelpMessage(const string& prompt="\t")
{
	info("commands:");
	info(string(HELP_CMD) + " -> display this help message", prompt);
	info(string(SEND_MSG_CMD) + " <user> <msg> -> messages <msg> to <user>", 
		prompt);
	info(string(CREATE_GROUP_CMD) + " <group> -> creates <group>", 
		prompt);
	info(string(JOIN_GROUP_CMD) + " <group> -> joins <group>", 
		prompt);
	info(string(SEND_GROUP_CMD) + " <group> <msg> -> sends <msg> to all"
		+ " users of <group>", prompt);
	info(string(SEND_FILE_CMD) + " <user> <file_path> -> sends file in"
		+ " <file_path> to <user>", prompt);
	info(string(WHO_CMD) + " -> displays users from chat and status", prompt);
	info(string(EXIT_CMD) + " -> exits chat", prompt);
}

/*
Cuts off spaces from left and right extremes of string.
*/
string trim(const string& str)
{
    size_t first, last;

	first = str.find_first_not_of(' ');
    if(first == string::npos)
        return "";
    last  = str.find_last_not_of(' ');

    return str.substr(first, last - first + 1);
}

/*
Converts every ASCII character to lower case.
*/
string lower(const string& str)
{
	string res(str);

	for(unsigned i=0; i<res.size(); i++)
		if(res[i] >= 'A' && res[i] <= 'Z')
			res[i] = res[i] - ('Z' - 'z');

    return res;
}

/*
Gets server response, shows it and takes appropriate action.
Returns a number less than zero if the program should exit.
*/
char handle(const string& answer, const string& prompt="[server] ")
{
	char header;
	char ret = OK;

	header = netToHostHeader(answer);

	switch(header)
	{
		case OK:
			info("OK", prompt);
			break;
		case SERVER_FULL:
			info("server cannot take this request at the moment", prompt);
			ret = EXIT;
			break;
		case MSG_QUEUED:
		{
			size_t msg_id;
			msg_id = netToHostMsgQueued(answer);
			info("message #" + to_string(msg_id).substr(0, HASH_CUT) 
				+ " queued to be sent!", prompt);
			break;
		}
		case MSG_SENT:
		{
			size_t msg_id;
			msg_id = netToHostMsgSent(answer);
			info("message #" + to_string(msg_id).substr(0, HASH_CUT) 
				+ " was sent to destination!", prompt);
			break;
		}
		case MSG_INCOMING:
		{
			Message msg = netToHostMsgIncoming(answer);
			info(msg.getContent(), "[msg from " + msg.getSrcUserName() + "] ");
			break;
		}
		case FILE_QUEUED:
		{
			size_t msg_id;
			msg_id = netToHostFileQueued(answer);
			info("file #" + to_string(msg_id).substr(0, HASH_CUT) 
				+ " queued to be sent!", prompt);
			break;
		}
		case FILE_SENT:
		{
			size_t msg_id;
			msg_id = netToHostFileSent(answer);
			info("file #" + to_string(msg_id).substr(0, HASH_CUT) 
				+ " was sent to destination!", prompt);
			break;
		}
		case FILE_INCOMING:
		{
			Message file = netToHostFileIncoming(answer);
			string title = file.getTitle();
			string base = title.substr(title.find_last_of("/\\") + 1);
			info("file '" + base + "'", 
				"[received from " + file.getSrcUserName() + "] ");
			info("use 'accept " + file.getSrcUserName() + ":" + base + "'"
				+ " to save file");
			files_mtx.lock();
			files[file.getSrcUserName() + ":" + base] = file.getContent();
			files_mtx.unlock();
			break;
		}
		case USERS_LIST:
		{
			string users_list = netToHostUsersList(answer);
			info("users list:", prompt);	
			cout << users_list;
			break;
		}
		case MSG_EXISTS:
			info("message already queued to be sent", prompt);
			break;
		case GROUP_EXISTS:
			info("group already exists", prompt);
			break;
		case USER_ALREADY_IN_GROUP:
			info("user already in group", prompt);
			break;
		case USER_ADDED_TO_GROUP:
			info("user successfully added to group", prompt);
			break;
		case GROUP_CREATED:
			info("group created successfully", prompt);
			break;
		case NO_GROUP_ERR:
			info("ERROR: group does not exist", prompt);
			ret = EXIT;
			break;
		case INVALID_REQUEST_ERR:
			info("ERROR: invalid request", prompt);
			ret = EXIT;
			break;
		case NOT_IN_GROUP_ERR:
			info("ERROR: user does not belong to group", prompt);
			ret = EXIT;
			break;
		case NO_MSG_DST_ERR:
			info("ERROR: destiny user does not exist", prompt);
			ret = EXIT;
			break;
		case USER_EXISTS_ERR:
			info("ERROR: cannot register, username already exists", prompt);
			ret = EXIT;
			break;
		default:
			info("unknown answer", prompt);
			break;
	}

	return ret;
}

/*
Continuously waits for a message from server and handles it.
*/
void observe(int sock, string prompt)
{
	NetAddr src;
	NetMessage msg;
	NetReceiver receiver;

	receiver = NetReceiver(sock);

	while(true)
	{
		//receiving server response
		msg = receiver.recv();
		if(msg.getErrCode() < 0)
			error("recv");	
		if(msg.getErrCode() == 0)
			break;

		//displaying server response
		src = msg.getSrcAddr();

		//handling answer
		handle(msg.getContent());

		cout << prompt << flush;
	}
}

/*
Tries to register with username to chat server.
Returns a number less than zero if operation was not successful.
*/
int registerUser(int sock, const string& name)
{
	int ret;
	NetMessage msg;
	NetReceiver receiver;

	receiver = NetReceiver(sock);

	//sending registering message to server
	info("registering user '" + name + "'...");
	ret = send(sock, hostToNetRegister(name));
	if(ret <= 0)
		error("send");	

	//receiving answer
	msg = receiver.recv();
	if(msg.getErrCode() < 0)
		error("recv");	

	return handle(msg.getContent());
}

/*
Parses input from user and returns appropriate request to be sent to server.
Returns an empty string if command is not valid.
*/
string cmdToNetMsg(const string& cmd, const string& user_name)
{
	vector<string> tokens;
	string arg_1;
	string arg_2;
	string arg_3;
	string str;

	tokens = split(cmd, " ");

	switch(tokens.size())
	{
		case 0:
			break;

		case 1:
			arg_1 = lower(tokens[0]);
			if(arg_1 == EXIT_CMD)
				str = hostToNetMsg(EXIT);
			if(arg_1 == WHO_CMD)
				str = hostToNetMsg(WHO);
			break;

		case 2:		
			arg_1 = lower(tokens[0]);
			arg_2 = tokens[1];
			if(arg_1 == JOIN_GROUP_CMD)
				str = hostToNetJoinGroup(arg_2);
			if(arg_1 == CREATE_GROUP_CMD)
				str = hostToNetCreateGroup(arg_2);
			break;

		//number of args >= 3
		default:
			arg_1 = lower(tokens[0]);
			arg_2 = tokens[1];
			if(arg_1 == SEND_MSG_CMD)
			{
				arg_3 = join(tokens, " ", 2);
				Message msg = Message(user_name, arg_2, arg_3);
				size_t msg_id = hash<Message>{}(msg);
				str = hostToNetSendMsg(msg);
				info("message #" + to_string(msg_id).substr(0, HASH_CUT)
					+ " to be sent!");
			}
			if(arg_1 == SEND_GROUP_CMD)
			{
				arg_3 = join(tokens, " ", 2);
				Message msg = Message(user_name, arg_2, arg_3);
				str = hostToNetSendGroup(arg_2, arg_3);
				size_t msg_id = hash<Message>{}(msg);
				info("message #" + to_string(msg_id).substr(0, HASH_CUT)
					+ " to be sent!");
			}
			if(arg_1 == SEND_FILE_CMD)
			{
				arg_3 = join(tokens, " ", 2);
				str = hostToNetSendFile(user_name, arg_2, arg_3);
				if(str.empty())
					info("could not read file '" + arg_3 + "'", "[ERROR]");
			}
			break;
	}

	return str;
}

/*
If a command to accept a file is issued, tries to save it.
*/
bool acceptFileIfRequested(const string& str)
{
	vector<string> tokens = split(str, " ");

	if(tokens.size() == 2 && lower(tokens[0]) == ACCEPT_FILE_CMD)
	{
		files_mtx.lock();
		if(files.find(tokens[1]) == files.end())
			info("no such file exists!");
		else
		{
			string file = files[tokens[1]];
			string base = split(tokens[1], ":")[1];
			string path = pwd() + "/" + base;
			ofstream out;
			//out.open(path, ofstream::binary);
			out.open(base, ofstream::out | ofstream::binary);
			info("file will be saved to '" + path + "'");
			if(out.is_open())
			{
				out.write(file.c_str(), file.size());
				info("file saved!");
			}
			else
				info("error saving file!");
		}
		files_mtx.unlock();
		return true;
	}

	return false;
}

/*
Connects to server, registers user and continuously gets commands from user
and sends request to server.
*/
void client(string& ip, unsigned short port, string& name)
{
	NetAddr server(ip, port);	
	int sock;
	int ret;
	NetMessage msg;
	NetAddr src;
	string cmd;
	string request;
	string prompt;
	thread thr;
	
	prompt = "$[" + name + "] ";

	if(server.getErrCode() < 0)
		error("NetAddr");

	sock = getSocket(SOCK_STREAM);	
	if(sock < 0)
		error("getSocket");

	//connecting to server
	if(connect(sock, server) < 0)
		error("connect");	
		
	info("connected to " + 
		server.getIp() + ":" + to_string(server.getPort()));

	if(registerUser(sock, name) != OK)
		return;
	
	//starting thread that receives server messages
	thr = thread(observe, sock, prompt);

	cout << prompt << flush;
	//main loop
	while(true)
	{
		//getting command from console
		getline(cin, cmd);

		if(acceptFileIfRequested(cmd))
		{
			cout << prompt << flush;
			continue;
		}
		if(lower(trim(cmd)) == HELP_CMD)
		{
			displayHelpMessage();
			cout << prompt << flush;
			continue;
		}

		//parsing command
		request = cmdToNetMsg(cmd, name);
		if(request.empty())
		{
			info("invalid command. use 'help' for more info.", prompt);
			cout << prompt << flush;
			continue;
		}

		//sending message
		ret = send(sock, request);
		if(ret < 0)
			error("error sending message to server");	
		if(ret == 0)
			break;

		if(lower(trim(cmd)) == EXIT_CMD)
			break;
	}

	if(thr.joinable())
		thr.join();

	close(sock);

	cout << "connection ended." << endl;
}

int main(int argc, char** argv)
{
	if(argc < 4)
	{
		info("usage: client <server_ip> <server_port> <name>");
		return 0;
	}

	//getting arguments from command line and starting main loop
	string ip(argv[1]), port(argv[2]), name(argv[3]);
	client(ip, (unsigned short)stoi(port), name);

	return 0;
}
