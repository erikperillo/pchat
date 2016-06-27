#include "net.h"
#include <cstring>
#include <fcntl.h>

NetAddr::NetAddr()
{
	memset((char*)&this->raw_addr, 0, sizeof(this->raw_addr));	
}

NetAddr::NetAddr(const std::string& ip, unsigned short port, int family)
{
	struct hostent* hp;

	hp = gethostbyname(ip.c_str());
	if(!hp)
		this->err_code = GETHOSTBYNAME_ERR;
	else
	{
		memset((char*)&this->raw_addr, 0, sizeof(this->raw_addr));	
		this->raw_addr.sin_family = family;
		this->raw_addr.sin_port = htons(port);
		memcpy((char*)&this->raw_addr.sin_addr, hp->h_addr, hp->h_length);
	}

	this->err_code = SUCCESS;
}

NetAddr::NetAddr(const sockaddr_in& raw_addr)
{
	memcpy((char*)&this->raw_addr, (char*)&raw_addr, sizeof(raw_addr));
}

NetAddr::NetAddr(const NetAddr& addr)
{
	memcpy((char*)&this->raw_addr, (char*)&addr.raw_addr, 
		sizeof(addr.raw_addr));
}

unsigned short NetAddr::getPort() const
{
	return ntohs(this->raw_addr.sin_port);
}

std::string NetAddr::getIp() const
{
	return std::string(inet_ntoa(this->raw_addr.sin_addr));
}

short NetAddr::getFamily() const
{
	return this->raw_addr.sin_family;
}

int NetAddr::getErrCode() const
{
	return this->err_code;
}

void NetAddr::setErrCode(int val)
{
	this->err_code = val;
}

struct sockaddr_in NetAddr::getRawAddr() const
{
	return this->raw_addr;
}

NetMessage::NetMessage(): err_code(0)
{;}

NetMessage::NetMessage(const NetAddr& src, const NetAddr& dst, 
	const std::string& content): err_code(0)
{
	this->src = src;
	this->dst = dst;
	this->content = content;
}

/*
NetMessage::NetMessage(const NetAddr& src, const NetAddr& dst, 
	char* buf, size_t buf_size)
{
	this->src = src;
	this->dst = dst;
	this->setContent(buf, buf_size);
}

void NetMessage::setContent(char* buf, size_t size)
{
	this->content.clear();
	for(unsigned i=0; i<size-1; i++)
		this->content.push_back((buf[i] == '\0')?NET_SEP:buf[i]);
}*/

NetAddr NetMessage::getSrcAddr() const
{
	return this->src;
}

NetAddr NetMessage::getDstAddr() const
{
	return this->dst;
}

std::string NetMessage::getContent() const
{
	return this->content;
}

int NetMessage::getErrCode() const
{
	return this->err_code;
}

void NetMessage::setErrCode(int val)
{
	this->err_code = val;
}

NetReceiver::NetReceiver(): socket(-1), buf_len(0)
{
	memset(this->buf, 0, MAX_BUF_LEN);
	this->buf[0] = '\0';
}

NetReceiver::NetReceiver(int socket): socket(socket), buf_len(0)
{
	memset(this->buf, 0, MAX_BUF_LEN);
	this->buf[0] = '\0';
}

NetMessage NetReceiver::recv(int flags)
{
	int ret;
	int i;
	char recv_buf[MAX_BUF_LEN];
	struct sockaddr_in src;
	struct sockaddr_in dst;
	socklen_t src_len = sizeof(src);
	socklen_t dst_len = sizeof(dst);
	NetMessage msg;
	std::string str;

	for(i=0; i<this->buf_len; i++)
		str.push_back(this->buf[i]);
	this->buf_len = 0;

	while(true)
	{
		ret = ::recv(this->socket, recv_buf, MAX_BUF_LEN, flags);		
		if(ret <= 0)
			return NetMessage(NetAddr(src), NetAddr(dst), std::string());	

		//if(buf[ret-1] == '\0')
		//	ret--;

		for(i=0; i<ret; i++)
			if(recv_buf[i] == NET_SEP)
			{
				memcpy(this->buf, recv_buf+i+1, ret-i-1);
				this->buf_len = ret-i-1;
				break;	
			}
			else
				str.push_back(recv_buf[i]);

		if(i < ret)
			break;
	}

	getpeername(socket, (struct sockaddr*)&src, &src_len);
	getsockname(socket, (struct sockaddr*)&dst, &dst_len);
	msg = NetMessage(NetAddr(src), NetAddr(dst), str);
	msg.setErrCode(SUCCESS);

	return msg;
}

int NetReceiver::getSocket()
{
	return this->socket;
}

int getSocket(int type, bool reuse, int family, int flags)
{
	int opt=1;
	int ret;
	int sock;

	sock = socket(family, type, flags);
	if(sock < 0 || !reuse)
		return sock;

	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, 
		sizeof(int));
	
	return (ret < 0)?ret:sock;
}

int bind(int socket, const NetAddr& addr)
{
	struct sockaddr_in raw_addr = addr.getRawAddr();
	return ::bind(socket, (struct sockaddr*)&raw_addr, sizeof(raw_addr));
}

int attend(int socket, int max_pending)
{
	return ::listen(socket, max_pending);
}

int connect(int socket, const struct sockaddr_in& addr)
{
	return ::connect(socket, (struct sockaddr*)&addr, sizeof(addr));
}

int connect(int socket, const NetAddr& addr)
{
	return connect(socket, addr.getRawAddr());
}

NetAddr accept(int socket)
{
	struct sockaddr_in raw_addr;
	socklen_t raw_addr_len = sizeof(raw_addr);
	NetAddr addr;
	int ret;

	ret = ::accept(socket, (struct sockaddr*)&raw_addr, &raw_addr_len);
	addr = NetAddr(raw_addr);	
	addr.setErrCode(ret);

	return addr;
}

int sendTo(int socket, const struct sockaddr_in& dst, const std::string& msg,
	int flags)
{
	return sendto(socket, msg.c_str(), msg.size()+1, flags,
		(struct sockaddr*)&dst, sizeof(dst));
}

int sendTo(int socket, const NetAddr& dst, const std::string& msg, int flags)
{
	return sendTo(socket, dst.getRawAddr(), msg, flags);
}

int sendTo(int socket, const NetMessage& msg, int flags)
{
	return sendTo(socket, msg.getDstAddr(), msg.getContent(), flags);	
}

int send(int socket, const std::string& msg, int flags)
{
	return ::send(socket, msg.c_str(), msg.size()+1, flags);
}

int send(int socket, const NetMessage& msg, int flags)
{
	return send(socket, msg.getContent(), flags);
}

int setBlocking(int socket, bool blocking)
{
	int flags;
	flags = fcntl(socket, F_GETFL, 0);
	if(flags < 0) 
		return flags;
	flags = blocking?(flags&~O_NONBLOCK):(flags|O_NONBLOCK);
	return (fcntl(socket, F_SETFL, flags) == 0)?true:false;
}

NetMessage recvFrom(int socket, int flags)
{
	int ret;
	char buf[MAX_BUF_LEN] = "\0";
	struct sockaddr_in src;
	struct sockaddr_in dst;
	socklen_t src_len = sizeof(src);
	socklen_t dst_len = sizeof(dst);
	NetMessage msg;

	ret = recvfrom(socket, buf, MAX_BUF_LEN, flags, 
		(struct sockaddr*)&src, &src_len);

	getsockname(socket, (struct sockaddr*)&dst, &dst_len);
	msg = NetMessage(NetAddr(src), NetAddr(dst), std::string(buf));	
	msg.setErrCode(ret);

	return msg;
}

NetMessage recv(int socket, int flags)
{
	int ret;
	char buf[MAX_BUF_LEN];
	struct sockaddr_in src;
	struct sockaddr_in dst;
	socklen_t src_len = sizeof(src);
	socklen_t dst_len = sizeof(dst);
	NetMessage msg;
	std::string str;

	ret = recv(socket, buf, MAX_BUF_LEN, flags);		
	buf[MAX_BUF_LEN-1] = '\0';

	getpeername(socket, (struct sockaddr*)&src, &src_len);
	getsockname(socket, (struct sockaddr*)&dst, &dst_len);
	msg = NetMessage(NetAddr(src), NetAddr(dst), std::string(buf));
	msg.setErrCode(ret);

	return msg;
}
