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

#define SERVER_NAME "127.0.0.1"
#define SERVER_PORT 7532
#define HASH_CUT 6
#define MAX_BUF 2048

#define REGISTER_CMD "register" 
#define EXIT_CMD "exit"
#define CREATE_GROUP_CMD "createg"
#define JOIN_GROUP_CMD "joing"
#define SEND_GROUP_CMD "sendg"
#define SEND_MSG_CMD "sendmsg"
#define SEND_FILE_CMD "sendf"
#define WHO_CMD "who"
#define HELP_CMD "help"

void error(const string& msg, int ret=1)
{
	perror(msg.c_str());
	exit(ret);
}

void info(const string& msg, const string& prompt="[chat] ")
{
	cout << prompt << msg << endl;
}

string pwd()
{
  char path[MAX_BUF];
  ssize_t size = readlink("/proc/self/exe", path, MAX_BUF);

  return string(path, (size>0)?size:0);
}

void displayHelpMessage(const string& prompt="\t")
{
	info("commands:");
	info(string(HELP_CMD) + " -> display this help message", prompt);
	info(string(SEND_MSG_CMD) + " <user> <msg> -> messages <msg> to <user>", 
		prompt);
	info(string(JOIN_GROUP_CMD) + " <group_name> -> joins <group_name>", 
		prompt);
	info(string(EXIT_CMD) + " -> exits chat", prompt);
}

string trim(const string& str)
{
    size_t first, last;

	first = str.find_first_not_of(' ');
    if(first == string::npos)
        return "";
    last  = str.find_last_not_of(' ');

    return str.substr(first, last - first + 1);
}

string lower(const string& str)
{
	string res(str);

	for(unsigned i=0; i<res.size(); i++)
		if(res[i] >= 'A' && res[i] <= 'Z')
			res[i] = res[i] - ('Z' - 'z');

    return res;
}

int handle(const string& answer, const string& prompt="[server] ")
{
	char header;
	int ret = 0;

	header = netToHostHeader(answer);

	switch(header)
	{
		case OK:
			info("OK", prompt);
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
			//cout << "title = " << title << endl;
			//cout << "base = " << base << endl;
			string path = pwd() + "/" + base;
			//cout << "path = " << path << endl;
			info("file '" + base + "'", 
				"[received from " + file.getSrcUserName() + "] ");
			ofstream out;
			//out.open(path, ofstream::binary);
			out.open(base, ofstream::out | ofstream::binary);
			if(out.is_open())
				info("file will be saved to '" + path + "'", 
					"[received from " + file.getSrcUserName() + "] ");
			out.write(file.getContent().c_str(), file.getContent().size());
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
			ret = -1;
			break;
		case INVALID_REQUEST_ERR:
			info("ERROR: invalid request", prompt);
			ret = -1;
			break;
		case NOT_IN_GROUP_ERR:
			info("ERROR: user does not belong to group", prompt);
			ret = -1;
			break;
		case NO_MSG_DST_ERR:
			info("ERROR: destiny user does not exist", prompt);
			ret = -1;
			break;
		case USER_EXISTS_ERR:
			info("ERROR: cannot register, username already exists", prompt);
			ret = -1;
			break;
		default:
			info("unknown answer", prompt);
			break;
	}

	return ret;
}

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
		//cout << "[" << src.getIp() << ":" << src.getPort() << "]"
		//	<< " " << msg.getContent() << endl;

		//handling answer
		//for(auto const& str: split(msg.getContent(), string(1, NET_SEP)))
		//	handle(sock, str);
		handle(msg.getContent());

		cout << prompt << flush;
	}
}

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

	if(connect(sock, server) < 0)
		error("connect");	
		
	info("connected to " + 
		server.getIp() + ":" + to_string(server.getPort()));

	if(registerUser(sock, name) < 0)
		return;
	
	thr = thread(observe, sock, prompt);

	cout << prompt << flush;
	while(true)
	{
		//getting command from console
		getline(cin, cmd);

		if(lower(cmd) == HELP_CMD)
		{
			displayHelpMessage();
			cout << prompt << flush;
			continue;
		}

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
		info("usage: client <ip> <port> <name>");
		return 0;
	}

	string ip(argv[1]), port(argv[2]), name(argv[3]);
	client(ip, (unsigned short)stoi(port), name);

	return 0;
}
