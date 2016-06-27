#ifndef __PROTOCOL_HEADER__
#define __PROTOCOL_HEADER__

#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "message.h"
#include "net.h"

#define FIELD_SEP ";"
#define MSG_SEP (std::string(1, NET_SEP))

enum messages: char
{
	SEND_MSG,
	CREATE_GROUP,
	JOIN_GROUP,
	GROUP_EXISTS,
	SEND_GROUP,
	GROUP_CREATED,
	USER_ADDED_TO_GROUP,
	USER_ALREADY_IN_GROUP,
	UNDEFINED,
	OK,
	HELP,
	WHO,
	USERS_LIST,
	REGISTER,
	EXIT,
	SEND_FILE,
	FILE_INCOMING,
	FILE_QUEUED,
	FILE_SENT,
	MSG_QUEUED,
	MSG_SENT,
	MSG_EXISTS,
	MSG_INCOMING,
	INVALID_REQUEST_ERR,
	NOT_IN_GROUP_ERR,
	NO_GROUP_ERR,
	NO_MSG_DST_ERR,
	USER_EXISTS_ERR
};

/*
class ProtocolFormatError
{
	private:
	std::string msg;

	public:
	ProtocolFormatError(const std::string& msg);
	std::string getMessage() const;
};*/

//std::string lower(const std::string& str);
//low-level methods
std::string sanitize(const std::string& str);
std::string desanitize(const std::string& str);
std::vector<std::string> split(const std::string& str, 
	const std::string& delim);

std::string initMsg(char message);
void addField(std::string& str, const std::string& field);
void endMsg(const std::string& str);
std::string join(const std::vector<std::string>& tokens, 
	const std::string& delim=" ", int pos=0);

//general methods
std::string hostToNetHeader(char message);
char netToHostHeader(const std::string& str);
std::string hostToNetMsg(char message);
std::string hostToNetMsg(char message, std::vector<std::string> args);
std::vector<std::string> netToHostMsg(std::string msg);

std::string hostToNetRegister(const std::string& name);
std::string netToHostRegister(const std::string& msg);

std::string hostToNetSendMsg(const std::string& msg);
std::string hostToNetSendMsg(const std::string& src_user_name, 
	const std::string& dst_user_name, const std::string& content);
std::string hostToNetSendMsg(const Message& msg);
Message netToHostSendMsg(const std::string& str);

std::string hostToNetJoinGroup(const std::string& group_name);
std::string netToHostJoinGroup(const std::string& str);

std::string hostToNetMsgQueued(std::size_t msg_id);
std::size_t netToHostMsgQueued(const std::string msg, int to_cut=-1);

std::string hostToNetMsgSent(std::size_t msg_id);
std::size_t netToHostMsgSent(const std::string msg, int to_cut=-1);

std::string hostToNetFileQueued(std::size_t msg_id);
std::size_t netToHostFileQueued(const std::string msg, int to_cut=-1);

std::string hostToNetFileSent(std::size_t msg_id);
std::size_t netToHostFileSent(const std::string msg, int to_cut=-1);

std::string hostToNetMsgIncoming(const Message& msg);
Message netToHostMsgIncoming(const std::string& msg);

std::string hostToNetCreateGroup(const std::string& name);
std::string netToHostCreateGroup(const std::string& msg);

std::string hostToNetSendGroup(const std::string& group_name, 
	const std::string& msg);
std::pair<std::string, std::string> netToHostSendGroup(const std::string& msg);

std::string fileToStr(const std::string& file_path);
std::string hostToNetSendFile(const std::string& src_user_name,
	const std::string& dst_user_name, const std::string& file_path);
Message netToHostSendFile(const std::string& msg);

std::string hostToNetFileIncoming(const Message& msg);
Message netToHostFileIncoming(const std::string& msg);

std::string hostToNetUsersList(const std::string& msg);
std::string netToHostUsersList(const std::string& msg);

#endif
