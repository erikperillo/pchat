#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include "net.h"
#include "protocol.h"

using namespace std;

#define SERVER_NAME "127.0.0.1"
#define SERVER_PORT 7532

void error(const string& msg, int ret=1)
{
	perror(msg.c_str());
	exit(ret);
}

void info(const string& msg, const string& prompt="[chat] ")
{
	cout << prompt << msg << endl;
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

string cmdToNetMsg(const string& cmd)
{
	vector<string> tokens;
	string msg(SEP);

	tokens = split(cmd, " ");
	for(auto const& tok: tokens)
		msg += sanitize(tok) + SEP;

	return msg;
}

int handle(int socket, const string& answer, const string& prompt="[server] ")
{
	if(answer == OK)	
		info("OK", prompt);
	else if(answer == HELP)
		displayHelpMessage();
	else if(answer == NO_SRC_USER_ERR)
		info("no source user for message registered");
	else if(answer == NO_DST_USER_ERR)
		info("no destiny user for message registered");
	else if(answer == MSG_EXISTS_ERR)
		info("message already registered");
	else if(answer == INVALID_CMD_ERR)
		info("invalid command", prompt);
	else if(answer == USER_EXISTS_ERR)
	{
		info("cannot register, username already exists", prompt);
		return -1;
	}
	else
		info("unknown answer", prompt);

	return 0;
}

void observe(int sock)
{
	NetAddr src;
	NetMessage msg;

	while(true)
	{
		//receiving server response
		msg = recv(sock);
		if(msg.getErrCode() < 0)
			error("recv");	
		if(msg.getErrCode() == 0)
			break;

		//displaying server response
		src = msg.getSrcAddr();
		cout << "[" << src.getIp() << ":" << src.getPort() << "]"
			<< " " << msg.getContent() << endl;

		//handling answer
		handle(sock, msg.getContent());

		cout << "$[" + string("erik") + "] " << flush;
	}
}

void client(string& ip, unsigned short port, string& name)
{
	NetAddr server(ip, port);	
	int sock;
	int ret;
	NetMessage msg;
	NetAddr src;
	string cmd;
	string str;
	thread thr;
	
	if(server.getErrCode() < 0)
		error("NetAddr");

	sock = getSocket(SOCK_DGRAM);	
	if(sock < 0)
		error("getSocket");

	if(connect(sock, server) < 0)
		error("connect");	

	info("connected to " + 
		server.getIp() + ":" + std::to_string(server.getPort()));

	//sending registering message to server
	info("registering user '" + name + "'...");
	str = string(REGISTER_CMD) + " " + name;
	ret = send(sock, str);
	if(ret <= 0)
		error("error sending message to server");	
	//receiving answer
	msg = recv(sock);
	if(msg.getErrCode() < 0)
		error("recv");	
	if(handle(sock, msg.getContent()) < 0)
		return;

	thr = thread(observe, sock);

	while(true)
	{
		//getting command from console
		cout << "$[" + name + "] " << flush;
		getline(cin, cmd);

		//converting command to message to be sent via net
		//str = cmdToNetMsg(cmd);
		str = cmd;
		//sending message
		ret = send(sock, str);
		if(ret < 0)
			error("error sending message to server");	
		if(ret == 0)
			break;
	}

	if(thr.joinable())
		thr.join();

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
