#ifndef __CHATCONTROLLER_HEADER__
#define __CHATCONTROLLER_HEADER__

#include "chat.h"
#include "chatview.h"
#include <limits>
#include <string>

enum err: signed int
{
	USER_NOT_FOUND = std::numeric_limits<int>::min(),
	GROUP_NOT_FOUND,
	USER_EXISTS,
	//GROUP_EXISTS,
	MESSAGE_NOT_FOUND,
	MESSAGE_EXISTS
};

#define SUCCESS 1

class ChatController
{
	private:
	Chat& model;	
	ChatView& view;

	public:
	ChatController(Chat& model, ChatView& view);
	int addGroup(const std::string& group_name);
	int delGroup(const std::string& group_name);
	int addUser(const User& user);
	int delUser(const std::string& user_name);
	int queueMessage(const Message& msg);
};

#endif
