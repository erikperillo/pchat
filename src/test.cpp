#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include "chat.h"
#include "user.h"
#include "group.h"
#include "message.h"
#include "chatview.h"
#include "net.h"
#include "protocol.h"
#include "hash.h"

using namespace std;

int users = 0;

inline void addUserCheck(Chat& chat, const string& name)
{
	if(!chat.addUser(User(name, NetAddr()), users++))
		cout << "user '" << name << "' could not be added to chat" << endl;
	else
		cout << "user '" << name << "' added to chat" << endl;
}

inline void addGroupCheck(Chat& chat, const string& name)
{
	if(!chat.addGroup(name))
		cout << "group '" << name << "' could not be added to chat" << endl;
	else
		cout << "group '" << name << "' added to chat" << endl;
}

inline void addUserToGroupCheck(Chat& chat, const string& user_name,
	const string& group_name)
{
	if(!chat.addUserToGroup(user_name, group_name))
		cout << "user '" << user_name << "' could not be added to group '" << 
		group_name << "'" << endl;
	else
		cout << "user '" << user_name << "' added to group '" << 
		group_name << "'" << endl;
}


inline void addMessageCheck(Chat& chat, const Message& msg)
{
	if(!chat.addMessage(msg))
		cout << "message could not be added to chat" << endl;
	else
		cout << "message added to chat" << endl;
}

void error(const std::string& msg, int ret_code=1)
{
	cout << "error: ";
	perror(msg.c_str());
	exit(ret_code);
}

void chatTest()
{
	Chat chat;

	addUserCheck(chat, "erik");
	addUserCheck(chat, "larissa");
	addUserCheck(chat, "erik");
	addUserCheck(chat, "mario");
	addUserCheck(chat, "jose");
	addUserCheck(chat, "leandro");

	addGroupCheck(chat, "safados");
	addGroupCheck(chat, "safados");
	addGroupCheck(chat, "pilantras");
	addGroupCheck(chat, "safados");
	addGroupCheck(chat, "gostosos");
	addGroupCheck(chat, "gostosos");

	Message msg1 = Message("", "", "");
	Message msg2 = Message("w", "", "");
	addMessageCheck(chat, msg1);
	addMessageCheck(chat, msg1);
	addMessageCheck(chat, msg2);
	addMessageCheck(chat, msg2);
	addMessageCheck(chat, msg1);

	addUserToGroupCheck(chat, "erik", "safados");
	addUserToGroupCheck(chat, "erik", "pilantras");
	addUserToGroupCheck(chat, "larissa", "pilantras");
	addUserToGroupCheck(chat, "erik", "pilantras");
	addUserToGroupCheck(chat, "erik", "kajdhf");
	addUserToGroupCheck(chat, "alkjdfh", "pilantras");
}

void chatViewTest()
{
	Chat chat;
	ChatView view(chat);

	addUserCheck(chat, "erik");
	addUserCheck(chat, "larissa");
	addUserCheck(chat, "mario");
	addUserCheck(chat, "jose");
	addUserCheck(chat, "leandro");

	addGroupCheck(chat, "safados");
	addGroupCheck(chat, "pilantras");
	addGroupCheck(chat, "safados");
	addGroupCheck(chat, "gostosos");

	Message msg1 = Message("", "", "");
	Message msg2 = Message("", "", "");
	addMessageCheck(chat, msg1);
	addMessageCheck(chat, msg2);

	addUserToGroupCheck(chat, "erik", "safados");
	addUserToGroupCheck(chat, "erik", "pilantras");
	addUserToGroupCheck(chat, "larissa", "pilantras");

	cout << "users:" << endl;
	view.printUsers();

	cout << "groups:" << endl;
	view.printGroups();

	view.printUsersFromGroup("pilantras");
}

#define SERVER_PORT 7532

void UDPTestServer()
{
	NetAddr addr("127.0.0.1", SERVER_PORT);	
	int sock;
	NetMessage msg;
	NetAddr src;
	
	if(addr.getErrCode() < 0)
		error("NetAddr");

	sock = getSocket(SOCK_DGRAM);	
	if(sock < 0)
		error("getSocket");

	if(bind(sock, addr) < 0)
	{
		perror("bind");
		error("bind");
	}

	while(true)
	{
		msg = recvFrom(sock);
		if(msg.getErrCode() < 0)
			error("recvFrom");	

		src = msg.getSrcAddr();
		cout << "[" << src.getIp() << ":" << src.getPort() << "] "
			<< msg.getContent() << endl;

		if(sendTo(sock, src, msg.getContent()) < 0)
			error("sendTo");
	}
}

#define MAX_NUM_THREADS 3
std::mutex mtx;
bool free_thread[MAX_NUM_THREADS];

void TCPTestServerMTService(int id, int sock)
{
	NetMessage msg;
	NetAddr src;

	while(true)
	{
		msg = recv(sock);
		if(msg.getErrCode() < 0)
			error("recv");	
		if(msg.getErrCode() == 0)
		{
			cout << "[" << id << "] connection ended." << endl;	
			return;
		}

		src = msg.getSrcAddr();
		cout << "[" << id << "]"
			<< "[" << src.getIp() << ":" << src.getPort() << "]"
			<< " " << msg.getContent() << endl;

		//if(send(new_sock, msg.getContent()) < 0)
		//	error("send");
	}	

	mtx.lock();
	free_thread[id] = true;
	mtx.unlock();
}


void TCPTestServerMT()
{
	NetAddr addr("127.0.0.1", SERVER_PORT);
	NetAddr conn;
	int sock;
	int new_sock;
	int i;
	std::thread threads[MAX_NUM_THREADS];

	for(int i=0; i<MAX_NUM_THREADS; i++)
		free_thread[i] = true;

	if(addr.getErrCode() < 0)
		error("addr");	

	sock = getSocket(SOCK_STREAM);	
	if(sock < 0)
		error("getSocket");
	
	if(bind(sock, addr) < 0)
		error("bind");
	if(attend(sock) < 0)
		error("attend");

	while(true)
	{
		conn = accept(sock);	
		new_sock = conn.getErrCode();
		if(new_sock < 0)
			error("accept");

		mtx.lock();
		for(i=0; i<MAX_NUM_THREADS; i++)
			if(free_thread[i])	
			{
				cout << "<new connection on " 
					<< conn.getIp() << ":" << conn.getPort() << ">" << endl;
				cout << "thread " << i << " is free" << endl;
				threads[i] = std::thread(TCPTestServerMTService, i, new_sock);
				free_thread[i] = false;	
				break;
			}
		mtx.unlock();

		if(i == MAX_NUM_THREADS)
			close(new_sock);
	}

	for(int i=0; i<MAX_NUM_THREADS; i++)
		threads[i].join();	
}

void TCPTestServer()
{
	NetAddr addr("127.0.0.1", SERVER_PORT);	
	int sock;
	int new_sock;
	NetMessage msg;
	NetAddr conn;
	
	if(addr.getErrCode() < 0)
		error("NetAddr");

	sock = getSocket(SOCK_STREAM);	
	if(sock < 0)
		error("getSocket");

	if(bind(sock, addr) < 0)
	{
		perror("bind");
		error("bind");
	}

	if(attend(sock) < 0)
		error("listen");

	conn = accept(sock);	
	new_sock = conn.getErrCode();
	if(new_sock < 0)
	{
		perror("accept");
		error("accept");
	}

	cout << "new connection on " << conn.getIp() << ":" << conn.getPort() 
		<< endl;

	while(true)
	{
		msg = recv(new_sock);
		if(msg.getErrCode() < 0)
			error("recv");	
		if(msg.getErrCode() == 0)
			error("connection ended.");	

		cout << msg.getContent() << endl;

		if(send(new_sock, msg.getContent()) < 0)
			error("send");
	}
}

void UDPTestClient()
{
	NetAddr addr("127.0.0.1", SERVER_PORT+1);	
	NetAddr dst("127.0.0.1", SERVER_PORT);
	NetAddr src;
	int sock;
	NetMessage msg;
	std::string str;
	
	if(addr.getErrCode() < 0)
		error("NetAddr1");
	if(dst.getErrCode() < 0)
		error("NetAddr2");

	sock = getSocket(SOCK_DGRAM);	
	if(sock < 0)
		error("getSocket");

	while(true)
	{
		cout << ">>> ";
		cin >> str;

		//NetMessage message(addr, dst, str);
		//if(sendTo(sock, message) < 0)
		if(sendTo(sock, dst, str) < 0)
			error("sendTo");	

		msg = recvFrom(sock);
		if(msg.getErrCode() < 0)
			error("recvFrom");

		src = msg.getSrcAddr();
		cout << "[" << src.getIp() << ":" << src.getPort() << "] "
			<< msg.getContent() << endl;
	}
}

void TCPTestClient()
{
	NetAddr dst("127.0.0.1", SERVER_PORT);	
	int sock;
	int ret;
	NetMessage msg;
	NetAddr conn;
	std::string str;
	
	if(dst.getErrCode() < 0)
		error("NetAddr");

	sock = getSocket(SOCK_STREAM);	
	if(sock < 0)
		error("getSocket");

	if(connect(sock, dst) < 0)
		error("connect");	

	cout << "connected to " << dst.getIp() << ":" << dst.getPort() << endl;

	while(true)
	{
		cout << ">>> ";
		cin >> str;

		ret = send(sock, str);
		if(ret < 0)
			error("send");	

		//msg = recv(sock);
		//if(msg.getErrCode() < 0)
		//	error("recv");

		//cout << msg.getContent() << endl;
	}
}

void protocolTest()
{
	Message msg;
	Message msg2;
	string str;
	string str2;

	str = "joels:on;";
	str2 = hostToNetRegister(str);
	cout << "hostToNetRegister(" << str << ") = " << str2 << endl;
	cout << "netToHostRegister(" << str2 << ") = " 
		<< netToHostRegister(str2) << endl;

	str = "igreja: pentecostal;;; jesus";
	str2 = hostToNetJoinGroup(str);
	cout << "hostToNetJoinGroup(" << str << ") = " << str2 << endl;
	cout << "netToHostJoinGroup(" << str2 << ") = " 
		<< netToHostJoinGroup(str2) << endl;

	msg = Message("er;;i;:;k", "ta::ina", "e ai: otaria; bls???");
	cout << "input (src/dst/content): "
		<< msg.getSrcUserName() << "/" << msg.getDstUserName() << "/" 
		<< msg.getContent() << endl;
	str = hostToNetSendMsg(msg);
	cout << "str = " << str << endl;
	msg2 = netToHostSendMsg(str);
	cout << "output (src/dst/content): "
		<< msg2.getSrcUserName() << "/" << msg2.getDstUserName() << "/" 
		<< msg2.getContent() << endl;

	str = fileToStr("/home/erik/media/images/random/garis.png");
	cout << "str = " << str << endl;
	cout << str.size() << endl;

	ofstream out("hue.png", ofstream::binary);
	out.write(str.c_str(), str.size());	
}

void hashTest()
{
	Message msg1("Paulo", "Henrique", "ey b0ss can i hab da pusy");
	Message msg2("erik", "ana paula", "huehuebrbr");
	Message msg3("erik", "ana paula", "uehuebrbr");
	Message msg4("erik", "ana paula", "uehuebrbr", "title");
	
	cout << "hash(msg1) = " << std::hash<Message>{}(msg1) << endl;
	cout << "hash(msg2) = " << std::hash<Message>{}(msg2) << endl;
	cout << "hash(msg3) = " << std::hash<Message>{}(msg3) << endl;
	cout << "hash(msg4) = " << std::hash<Message>{}(msg4) << endl;
}

int main()
{
	#ifdef MODEL
	chatTest();
	#endif
	#ifdef VIEW
	chatViewTest();
	#endif
	#ifdef UDPSERVER
	UDPTestServer();
	#endif
	#ifdef UDPCLIENT
	UDPTestClient();
	#endif
	#ifdef TCPSERVER
	TCPTestServer();
	#endif
	#ifdef TCPCLIENT
	TCPTestClient();
	#endif
	#ifdef PROTOCOL
	protocolTest();
	#endif
	#ifdef TCPSERVERMT
	TCPTestServerMT();
	#endif
	#ifdef HASH
	hashTest();
	#endif

	return 0;
}
