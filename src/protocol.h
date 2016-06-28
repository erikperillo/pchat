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

/*
This module is used for formatting messages to/from net from/to host. 
It also holds information about every message that can be exchanged between 
client/server.
Any message between client/server must be manipulated by this module first.
*/

//field separator in messages
#define FIELD_SEP ";"
#define MSG_SEP (std::string(1, NET_SEP))

//messages that can be exchanged between server and client
enum messages: char
{
	//send a message to some user
	SEND_MSG,
	//create a chat group
	CREATE_GROUP,
	//join some chat group
	JOIN_GROUP,
	//group already exists
	GROUP_EXISTS,
	//send some message to some group
	SEND_GROUP,
	//success message for create group 
	GROUP_CREATED,
	//user was added to group graciously
	USER_ADDED_TO_GROUP,
	//user is already in group
	USER_ALREADY_IN_GROUP,
	//unknown message
	UNDEFINED,
	//success
	OK,
	//request for help
	HELP,
	//list users
	WHO,
	//a list of users is coming
	USERS_LIST,
	//register some user
	REGISTER,
	//exit chat
	EXIT,
	//server is full
	SERVER_FULL,
	//send some file to someone
	SEND_FILE,
	//there is a file coming
	FILE_INCOMING,
	//file was queued to be sent to destination
	FILE_QUEUED,
	//file was sent to destination
	FILE_SENT,
	//message was queued to be sent to destination
	MSG_QUEUED,
	//message was sent to destination
	MSG_SENT,
	//message is already queued to be sent
	MSG_EXISTS,
	//there is a message incoming
	MSG_INCOMING,
	//request from client is invalid
	INVALID_REQUEST_ERR,
	//user is not in group
	NOT_IN_GROUP_ERR,
	//no such group exists
	NO_GROUP_ERR,
	//no message destination exists
	NO_MSG_DST_ERR,
	//user already exists
	USER_EXISTS_ERR
};

//validates input string to be sent via net
std::string sanitize(const std::string& str);
//converts message from net back to its original form
std::string desanitize(const std::string& str);
//splits a string by character delim
std::vector<std::string> split(const std::string& str, 
	const std::string& delim);
//returns a netstring with message as header
std::string initMsg(char message);
//adds a field to netstring
void addField(std::string& str, const std::string& field);
//ends valid netstring
void endMsg(const std::string& str);
//joins a vector of string by delim, starting at pos
std::string join(const std::vector<std::string>& tokens, 
	const std::string& delim=" ", int pos=0);

//message-specific methods:
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

std::string hostToNetSendFile(const std::string& src_user_name,
	const std::string& dst_user_name, const std::string& file_path);
Message netToHostSendFile(const std::string& msg);

std::string hostToNetFileIncoming(const Message& msg);
Message netToHostFileIncoming(const std::string& msg);

std::string hostToNetUsersList(const std::string& msg);
std::string netToHostUsersList(const std::string& msg);

#endif
