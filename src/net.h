#ifndef __NET_HEADER__
#define __NET_HEADER__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <limits>

#define SUCCESS 1
#define MAX_BUF_LEN 8192
#define NET_SEP ':'

enum net_err: signed int
{
	GETHOSTBYNAME_ERR = std::numeric_limits<int>::min()
};

class NetAddr
{
	private:
	int err_code;
	struct sockaddr_in raw_addr;

	public:
	NetAddr();
	NetAddr(const std::string& ip, unsigned short port, int family=AF_INET);
	NetAddr(const sockaddr_in& raw_addr);
	NetAddr(const NetAddr& addr);
	unsigned short getPort() const;
	std::string getIp() const;	
	short getFamily() const;
	int getErrCode() const;
	void setErrCode(int val);
	struct sockaddr_in getRawAddr() const;
};

class NetMessage
{
	private:
	int err_code;
	NetAddr src;
	NetAddr dst;	
	std::string content;
	//void setContent(char* buf, size_t size);

	public:
	NetMessage();
	NetMessage(const NetAddr& src, const NetAddr& dst, 
		const std::string& content);	
	//NetMessage(const NetAddr& src, const NetAddr& dst, 
	//	char* buf, size_t buf_size);
	NetAddr getSrcAddr() const;
	NetAddr getDstAddr() const;
	std::string getContent() const;
	int getErrCode() const;
	void setErrCode(int val);
};

class NetReceiver
{
	private:
	int socket;
	int buf_len;
	char buf[MAX_BUF_LEN];

	public:
	NetReceiver();
	NetReceiver(int socket);
	NetMessage recv(int flags=0);
	int getSocket();
};

int getSocket(int type, bool reuse=true, int family=PF_INET, int flags=0);
int bind(int socket, const NetAddr& addr);
int attend(int socket, int max_pending=5);
int connect(int socket, const struct sockaddr_in& addr);
int connect(int socket, const NetAddr& addr);
NetAddr accept(int socket);
int sendTo(int socket, const struct sockaddr_in& dst, const std::string& msg,
	int flags=0);
int sendTo(int socket, const NetAddr& dst, const std::string& msg, int flags=0);
int sendTo(int socket, const NetMessage& msg, int flags=0);
int send(int socket, const std::string& msg, int flags=0);
int send(int socket, const NetMessage& msg, int flags=0);
int setBlocking(int socket, bool blocking);

NetMessage recvFrom(int socket, int flags=0);
NetMessage recv(int socket, int flags=0);

#endif
