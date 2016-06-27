#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include "net.h"
#include "protocol.h"
#include "chat.h"
#include "chatview.h"
#include "chatcontroller.h"

using namespace std;

#define ONLINE_GROUP " online"
#define OFFLINE_GROUP "offline"
#define SERVER_NAME "127.0.0.1"
#define SERVER_PORT 13254
#define MSG_LOOP_TIME_MS 1618
#define MAX_NUM_THREADS 4

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

void messagesLoop()
{
	vector<size_t> messages_ids;
	Message msg;
	Group online;
	string src;
	string dst;
	string net_msg;
	int sock;
	size_t msg_id;

	while(true)
	{
		chat_mtx.lock();	
		messages_ids = chat.getMessagesIds();		
		for(auto const& id: messages_ids)
		{
			online = chat.getGroup(ONLINE_GROUP);
			msg = chat.getMessage(id);
			src = msg.getSrcUserName();
			dst = msg.getDstUserName();
			bool is_file = msg.hasTitle();
			if(online.has(src) && online.has(dst))
			{
				sock = chat.getSocketFromUser(dst);	
				if(is_file)
					net_msg = hostToNetFileIncoming(msg);
				else
					net_msg = hostToNetMsgIncoming(msg);
				//cout << "sock1 = " << sock << endl;
				//sending message to destiny user
				if(send(sock, net_msg) < 0)
				{
					cout << "error sending message from " + src + " to " + dst
						<< ":" << endl;
					perror("");
				}
				else
				{
					msg_id = hash<Message>{}(msg);
					chat.delMessage(msg_id);
					sock = chat.getSocketFromUser(src);	
					//cout << "sock2 = " << sock << endl;
					if(is_file)
						net_msg = hostToNetFileSent(msg_id);
					else
						net_msg = hostToNetMsgSent(msg_id);
					//notificating source user
					if(send(sock, net_msg) < 0)
					{
						cout << "error sending " << (is_file?"file":"message")
							<< " from " << src << " to " << dst << ":" << endl;
						perror("");
					}
				}
			}
		}
		chat_mtx.unlock();	

		this_thread::sleep_for(chrono::milliseconds(MSG_LOOP_TIME_MS));
	}
}

int handle(int socket, const string& str, const User& user)
{
	char header;
	string answer;
	int ret = 0;

	header = netToHostHeader(str);

	switch(header)
	{
		case EXIT:
		{
			answer = hostToNetMsg(OK);
			ret = -1;
			break;
		}
		case WHO:
		{
			stringstream ss;
			vector<string> header = {"user", "status"};

			chat_mtx.lock();
			ChatView view(chat, ss);
			view.printUsersFromGroup(ONLINE_GROUP, header);	
			view.printUsersFromGroup(OFFLINE_GROUP, false);	
			chat_mtx.unlock();
			
			answer = hostToNetUsersList(ss.str());
			break;
		}
		case SEND_MSG:
		{
			Message msg = netToHostSendMsg(str);		
			/*cout << "src | dst | content: " 
				<< msg.getSrcUserName() << " | " 
				<< msg.getDstUserName() << " | "
				<< msg.getContent() << endl;*/
			if(msg.getSrcUserName().empty())
			{
				answer = hostToNetMsg(INVALID_REQUEST_ERR);	
				break;
			}

			bool has_user, add_msg=true;

			chat_mtx.lock();
			has_user = chat.hasUser(msg.getDstUserName());
			if(has_user)
				add_msg = chat.addMessage(msg);
			chat_mtx.unlock();

			if(!has_user)
				answer = hostToNetMsg(NO_MSG_DST_ERR);
			else if(!add_msg)
				answer = hostToNetMsg(MSG_EXISTS);
			else
			{
				size_t msg_id;
				msg_id = hash<Message>{}(msg);
				//cout << msg_id << endl;
				answer = hostToNetMsgQueued(msg_id);
			}
			break;
		}
		case SEND_FILE:
		{
			Message msg = netToHostSendFile(str);		
			/*cout << "src | dst | content: " 
				<< msg.getSrcUserName() << " | " 
				<< msg.getDstUserName() << " | "
				<< msg.getContent() << endl;*/
			if(msg.getSrcUserName().empty())
			{
				answer = hostToNetMsg(INVALID_REQUEST_ERR);	
				break;
			}

			bool has_user, add_msg=true;

			chat_mtx.lock();
			has_user = chat.hasUser(msg.getDstUserName());
			if(has_user)
				add_msg = chat.addMessage(msg);
			chat_mtx.unlock();

			if(!has_user)
				answer = hostToNetMsg(NO_MSG_DST_ERR);
			else if(!add_msg)
				answer = hostToNetMsg(MSG_EXISTS);
			else
			{
				size_t msg_id;
				msg_id = hash<Message>{}(msg);
				//cout << msg_id << endl;
				answer = hostToNetFileQueued(msg_id);
			}
			break;
		}
		case CREATE_GROUP:
		{
			string group_name = netToHostCreateGroup(str);
			if(group_name.empty())
			{
				answer = hostToNetMsg(INVALID_REQUEST_ERR);
				break;
			}
			bool has_group;
			chat_mtx.lock();
			has_group = chat.hasGroup(group_name);
			chat.addGroup(group_name);
			chat_mtx.unlock();

			if(has_group)
				answer = hostToNetMsg(GROUP_EXISTS);
			else
				answer = hostToNetMsg(GROUP_CREATED);	
			break;
		}	
		case JOIN_GROUP:
		{
			string group_name = netToHostJoinGroup(str);
			if(group_name.empty())
			{
				answer = hostToNetMsg(INVALID_REQUEST_ERR);
				break;
			}

			bool has_group, add_user;
			chat_mtx.lock();
			has_group = chat.hasGroup(group_name);
			add_user = chat.addUserToGroup(user.getName(), group_name);
			chat_mtx.unlock();

			if(!has_group)
				answer = hostToNetMsg(NO_GROUP_ERR);	
			else if(!add_user)
				answer = hostToNetMsg(USER_ALREADY_IN_GROUP);	
			else
				answer = hostToNetMsg(USER_ADDED_TO_GROUP);
			break;
		}
		case SEND_GROUP:
		{
			pair<string, string> group_msg = netToHostSendGroup(str);	
			if(group_msg.first.empty())
			{
				answer = hostToNetMsg(INVALID_REQUEST_ERR);
				break;
			}

			Group group;
			bool in_group, has_group;
			string user_name = user.getName();
			string group_name = group_msg.first;
			string group_msg_c = group_msg.second;

			chat_mtx.lock();
			has_group = chat.hasGroup(group_name);
			if(has_group)
			{
				group = chat.getGroup(group_name);
				in_group = group.has(user_name);
				if(in_group)
					for(auto const& name: group.getUsersNames())
						if(name != user_name)
						{
							Message msg = Message(user_name, name, group_msg_c);
							chat.addMessage(msg);
						}
			}
			chat_mtx.unlock();

			if(has_group)
			{
				if(!in_group)
					answer = hostToNetMsg(NOT_IN_GROUP_ERR);
				else
				{
					size_t msg_id;
					Message msg = Message(user_name, group_name, group_msg_c);
					msg_id = hash<Message>{}(msg);
					//cout << msg_id << endl;
					answer = hostToNetMsgQueued(msg_id);
				}
			}
			else
				answer = hostToNetMsg(NO_GROUP_ERR);

			break;
		}
		default:
			answer = hostToNetMsg(INVALID_REQUEST_ERR);
			break;
	}

	if(send(socket, answer) < 0)
		error("send");

	return ret;
}

User registerUser(NetReceiver& receiver)
{
	User user;
	vector<string> tokens;
	string user_name;
	NetMessage msg;
	NetAddr src;
	bool ret;
	char header;

	msg = receiver.recv();
	if(msg.getErrCode() < 0)
		error("recv");	

	header = netToHostHeader(msg.getContent());
	if(header != REGISTER)
		return User(hostToNetHeader(INVALID_REQUEST_ERR), NetAddr());
	
	user_name = netToHostRegister(msg.getContent());
	if(user_name.empty())
		return User(hostToNetHeader(INVALID_REQUEST_ERR), NetAddr());

	//trying to add user to chat
	chat_mtx.lock();
	ret = chat.addUser(User(user_name, src), receiver.getSocket());
	user = chat.getUser(user_name);
	chat_mtx.unlock();

	if(!ret)
	{
		//user is being used by another host
		if(user.getAddr().getIp() != src.getIp())
			return User(hostToNetHeader(USER_EXISTS_ERR), NetAddr());
		//user is getting online again
		chat_mtx.lock();
		chat.delUser(user_name);
		user = User(user_name, src);
		chat.addUser(user, receiver.getSocket());
		chat.delUserFromGroup(user_name, OFFLINE_GROUP);
		chat.addUserToGroup(user_name, ONLINE_GROUP);
		chat_mtx.unlock();
		return user;
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
void userInteraction(int id, int sock)
{
	NetAddr src;
	NetMessage msg;
	string answer;
	User user;
	NetReceiver receiver;
	char header;

	//creating receiver object
	receiver = NetReceiver(sock);

	//registering user
	user = registerUser(receiver);
	header = netToHostHeader(msg.getContent());
	if(header == USER_EXISTS_ERR || header == INVALID_REQUEST_ERR)
		answer = hostToNetMsg(header);
	else
		answer = hostToNetMsg(OK);
	if(send(sock, answer) < 0)
		error("send");	

	//main loop
	if(answer == hostToNetMsg(OK))
	{
		while(true)
		{
			//receiving message from client
			msg = receiver.recv();
			if(msg.getErrCode() < 0)
				error("recv");	
			if(msg.getErrCode() == 0)
				break;

			//displaying message
			src = msg.getSrcAddr();
			//cout << "[" << id << "]"
			//	<< "[" << src.getIp() << ":" << src.getPort() << "]"
			//	<< " " << msg.getContent() << endl;

			//handling request
			if(handle(sock, msg.getContent(), user) < 0)
				break;
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
	sock = getSocket(SOCK_STREAM);	
	if(sock < 0)
		error("getSocket");
	
	//binding and listening for incoming connections
	if(bind(sock, addr) < 0)
		error("bind");
	if(listen(sock, MAX_NUM_THREADS) < 0)
		error("listen");

	cout << "started server" << endl;
	//main loop
	while(true)
	{
		//getting new connection
		conn = accept(sock);	
		conn_sock = conn.getErrCode();
		if(conn_sock < 0)
			error("accept");

		free_thread_mtx.lock();
		for(i=0; i<MAX_NUM_THREADS && !free_thread[i]; i++);
		free_thread_mtx.unlock();
		if(i < MAX_NUM_THREADS)
		{
			if(threads[i].joinable())
				threads[i].join();

			cout << "<new connection on " 
				<< conn.getIp() << ":" << conn.getPort() << ">" << endl;
			
			free_thread_mtx.lock();
			free_thread[i] = false;	
			free_thread_mtx.unlock();
			threads[i] = std::thread(userInteraction, i, conn_sock);
		}
	}

	close(sock);
}

int main()
{
	thread msg_thread;

	//creating first groups
	chat.addGroup(ONLINE_GROUP);
	chat.addGroup(OFFLINE_GROUP);

	//creating thread to handle messages
	msg_thread = thread(messagesLoop);	

	//main server loop
	serverLoop();

	msg_thread.join();

	return 0;
}
