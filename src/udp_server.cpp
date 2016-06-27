#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include "net.h"
#include "protocol.h"
#include "chat.h"
#include "chatview.h"
#include "chatcontroller.h"

using namespace std;

#define ONLINE_GROUP "online"
#define OFFLINE_GROUP "offline"
#define SERVER_NAME "127.0.0.1"
#define SERVER_PORT 13254
#define MAX_NUM_THREADS 1

//mutexes
std::mutex free_thread_mtx;
std::mutex chat_mtx;
//indicates which threads are free to be used
bool free_thread[MAX_NUM_THREADS];
//chat
Chat chat;

void error(const std::string& msg, int ret=1)
{
	perror(msg.c_str());
	exit(ret);
}

vector<string> preProcess(const std::string& raw_msg)
{
	return split(raw_msg, " ");
}

int handle(int socket, const string& str, const User& user)
{
	vector<string> tokens;
	string answer;
	string cmd;
	string arg_1;
	string arg_2;

	tokens = preProcess(str);

	switch(tokens.size())
	{
		case 0:
			answer = INVALID_CMD_ERR;
			break;

		case 1:
			cmd = lower(tokens[0]);
			if(cmd == EXIT_CMD)
				answer = OK;
			if(cmd == HELP_CMD)
				answer = HELP;
			else
				answer = INVALID_CMD_ERR;
			break;

		case 2:		
			cmd = lower(tokens[0]);
			arg_1 = tokens[1];
			if(cmd == JOIN_GROUP_CMD)
				answer = OK;
			break;

		//number of args >= 3
		default:
			cmd = lower(tokens[0]);
			arg_1 = tokens[1];
			if(cmd == SEND_MSG_CMD)
			{
				string msg_content = join(tokens, " ", 2);
				Message msg(user.getName(), arg_1, msg_content);
				cout << "message: " << msg.getContent() << endl;

				chat_mtx.lock();
				if(!chat.hasUser(msg.getSrcUserName()))
					answer = NO_SRC_USER_ERR;
				else if(!chat.hasUser(msg.getDstUserName()))
					answer = NO_DST_USER_ERR;
				else if(!chat.addMessage(msg))
					answer = MSG_EXISTS_ERR;
				else
					answer = MSG_QUEUED;
				chat_mtx.unlock();
			}
			else
				answer = INVALID_CMD_ERR;
			break;
	}

	if(send(socket, answer) < 0)
		error("send");	
	if(send(socket, "hur brbrb\n") < 0)
		error("send");	

	return 0;
}

User registerUser(int sock)
{
	User user;
	vector<string> tokens;
	string user_name;
	NetMessage msg;
	NetAddr src;
	bool ret;

	msg = recv(sock);
	if(msg.getErrCode() < 0)
		error("recv");	

	tokens = preProcess(msg.getContent());
	if(tokens.size() < 2 || lower(tokens[0]) != REGISTER_CMD)
		return User(INVALID_CMD_ERR, NetAddr());
	
	src = msg.getSrcAddr();
	user_name = tokens[1];
	//trying to add user to chat
	chat_mtx.lock();
	ret = chat.addUser(User(user_name, src));
	user = chat.getUser(user_name);
	chat_mtx.unlock();

	if(!ret)
	{
		//user is being used by another host
		if(user.getAddr().getIp() != src.getIp())
			return User(USER_EXISTS_ERR, NetAddr());
		//user is getting online again
		chat_mtx.lock();
		chat.delUserFromGroup(user_name, OFFLINE_GROUP);
		chat.addUserToGroup(user_name, ONLINE_GROUP);
		chat_mtx.unlock();
		return User(user_name, src);
	}
	else
	{
		chat_mtx.lock();
		chat.addUserToGroup(user_name, ONLINE_GROUP);
		chat_mtx.unlock();
	}

	return user;
}

/*
The worker thread.
Interacts with each client, providing necessary services
*/
void userInteraction(int id, int sock, NetAddr src)
{
	NetMessage msg;
	string answer = OK;
	//string answer;
	User user;

	//registering user
	/*user = registerUser(sock);
	if(user.getName() == USER_EXISTS_ERR || user.getName() == INVALID_CMD_ERR)
		answer = user.getName();
	else
		answer = OK;
	if(send(sock, answer) < 0)
		error("send");	*/

	//main loop
	if(answer == OK)
	{
		while(true)
		{
			//receiving message from client
			msg = recvFrom(sock);
			if(msg.getErrCode() < 0)
				error("recv");	
			if(msg.getErrCode() == 0)
				break;
			cout << "boss" << endl;

			//displaying message
			cout << "[" << id << "]"
				<< "[" << src.getIp() << ":" << src.getPort() << "]"
				<< " " << msg.getContent() << endl;

			if(sendTo(sock, msg.getSrcAddr(), msg.getContent()) < 0)
				error("sendTo");
			if(sendTo(sock, msg.getSrcAddr(), "ey b0ss") < 0)
				error("sendTo");
			if(sendTo(sock, msg.getSrcAddr(), "edjfh") < 0)
				error("sendTo");
			//handling request
			//handle(sock, msg.getContent(), user);
		}	

		//unregistering user
		chat_mtx.lock();
		chat.delUserFromGroup(user.getName(), ONLINE_GROUP);
		chat.addUserToGroup(user.getName(), OFFLINE_GROUP);
		chat_mtx.unlock();
	}

	cout << "[" << id << "] connection ended." << endl;	

	close(sock);
	//marking itself as free at the end
	free_thread_mtx.lock();
	free_thread[id] = true;
	free_thread_mtx.unlock();
}

/*
Main server loop.
Receives incoming connections and dispatches them to service loops.
*/
void serverLoop()
{
	NetAddr addr(SERVER_NAME, SERVER_PORT);
	NetAddr conn;
	std::thread threads[MAX_NUM_THREADS];
	int sock;
	int conn_sock;
	int i;

	//marking every thread as free
	for(i=0; i<MAX_NUM_THREADS; i++)
		free_thread[i] = true;

	if(addr.getErrCode() < 0)
		error("addr");	

	//getting socket for TCP connection
	sock = getSocket(SOCK_DGRAM);	
	if(sock < 0)
		error("getSocket");
	
	//binding and listening for incoming connections
	if(bind(sock, addr) < 0)
		error("bind");
	//if(listen(sock, MAX_NUM_THREADS) < 0)
	//	error("listen");

	cout << "started server" << endl;
	//main loop
	while(true)
	{
		//getting new connection
		/*
		conn = accept(sock);	
		conn_sock = conn.getErrCode();
		if(conn_sock < 0)
			error("accept");
		*/

		free_thread_mtx.lock();
		for(i=0; i<MAX_NUM_THREADS && !free_thread[i]; i++);
		free_thread_mtx.unlock();
		if(i < MAX_NUM_THREADS)
		{
			if(threads[i].joinable())
				threads[i].join();

			//cout << "<new connection on " 
			//	<< conn.getIp() << ":" << conn.getPort() << ">" << endl;
			
			free_thread[i] = false;	
			threads[i] = std::thread(userInteraction, i, sock, conn);
		}
	}

	//waiting for threads to finish
	//for(i=0; i<MAX_NUM_THREADS; i++)
	//	threads[i].join();	
}

int main()
{
	//creating first groups
	chat.addGroup(ONLINE_GROUP);
	chat.addGroup(OFFLINE_GROUP);

	serverLoop();

	return 0;
}
