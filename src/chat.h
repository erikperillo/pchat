#ifndef __CHAT_HEADER__
#define __CHAT_HEADER__

#include <vector>
#include <map>
#include "group.h"
#include "message.h"
#include "user.h"
#include "hash.h"
#include "elementnotfound.h"

class Chat
{
	private:
	std::map<std::size_t, Message> messages;
	std::map<int, User> users;
	std::vector<Group> groups;

	public:
	Chat();
	//groups methods
	bool addGroup(const std::string& group_name);
	bool delGroup(const std::string& group_name);
	bool hasGroup(const std::string& group_name);
	Group getGroup(const std::string& group_name) const throw(ElementNotFound);
	std::vector<std::string> getGroupsNames() const;

	//users methods
	bool addUser(const User& user, int socket=0);
	bool delUser(const std::string& user_name);
	bool hasUser(const std::string& user_name);
	User getUser(const std::string& user_name) const throw(ElementNotFound);
	std::vector<std::string> getUsersNames() const;
	bool addUserToGroup(const std::string& user_name, 
		const std::string& group_name);
	bool delUserFromGroup(const std::string& user_name, 
		const std::string& group_name);
	int getSocketFromUser(const std::string& user_name);

	//messages methods
	bool addMessage(const Message& msg);
	bool delMessage(const std::size_t& msg_id);
	bool hasMessage(const std::size_t& msg_id);
	Message getMessage(const std::size_t& msg_id) const throw(ElementNotFound);
	std::vector<std::size_t> getMessagesIds() const;
};

#endif
