#include "chat.h"

Chat::Chat()
{;}

bool Chat::addGroup(const std::string& group_name)
{
	//checking to see if group is already inserted
	for(auto const& g: this->groups)
		if(g.getName() == group_name)
			return false;

	this->groups.push_back(Group(group_name));
	return true;
}

bool Chat::delGroup(const std::string& group_name)
{
	for(unsigned i=0; i<this->groups.size(); i++)
		if(this->groups[i].getName() == group_name)
		{
			this->groups.erase(this->groups.begin() + i);
			return true;
		}

	return false;
}

bool Chat::hasGroup(const std::string& group_name)
{
	for(auto const& g: this->groups)
		if(g.getName() == group_name)
			return true;

	return false;
}

Group Chat::getGroup(const std::string& group_name) const throw(ElementNotFound)
{
	for(auto const& g: this->groups)
		if(g.getName() == group_name)
			return g;

	throw ElementNotFound("group " + group_name + " could not be found");
}

std::vector<std::string> Chat::getGroupsNames() const
{
	std::vector<std::string> names;

	for(auto const& g: this->groups)
		names.push_back(g.getName());

	return names;
}

bool Chat::addUser(const User& user, int socket)
{
	//checking to see if group is already inserted
	for(auto& u: this->users)
		if(u.second.getName() == user.getName())
			return false;

	this->users[socket] = user;
	return true;
}

bool Chat::delUser(const std::string& user_name)
{
	for(auto const& u: this->users)
		if(u.second.getName() == user_name)
		{
			this->users.erase(u.first);
			return true;
		}

	return false;
}

bool Chat::hasUser(const std::string& user_name)
{
	for(auto const& u: this->users)
		if(u.second.getName() == user_name)
			return true;

	return false;
}

User Chat::getUser(const std::string& user_name) const throw(ElementNotFound)
{
	for(auto const& u: this->users)
		if(u.second.getName() == user_name)
			return u.second;

	throw ElementNotFound("user " + user_name + " could not be found");
}

std::vector<std::string> Chat::getUsersNames() const
{
	std::vector<std::string> names;

	for(auto const& u: this->users)
		names.push_back(u.second.getName());

	return names;
}

int Chat::getSocketFromUser(const std::string& user_name)
{
	for(auto const& u: this->users)
		if(u.second.getName() == user_name)
			return u.first;

	return -1;
}

bool Chat::addUserToGroup(const std::string& user_name,
		const std::string& group_name)
{
	if(!this->hasUser(user_name))
		return false;

	for(auto& g: this->groups)
		if(g.getName() == group_name)
			return g.addUser(user_name);

	return false;
}

bool Chat::delUserFromGroup(const std::string& user_name,
		const std::string& group_name)
{
	for(auto& g: this->groups)
		if(g.getName() == group_name)
			return g.delUser(user_name);

	return false;
}

bool Chat::addMessage(const Message& msg)
{
	std::size_t msg_id = std::hash<Message>{}(msg);
	auto it = this->messages.find(msg_id);

	if(it != this->messages.end())
		return false;

	this->messages[msg_id] = msg;

	return true;
}

bool Chat::delMessage(const std::size_t& msg_id)
{
	auto it = this->messages.find(msg_id);

	if(it == this->messages.end())
		return false;

	this->messages.erase(msg_id);

	return true;
}

bool Chat::hasMessage(const std::size_t& msg_id)
{
	return this->messages.find(msg_id) != this->messages.end();
}

Message Chat::getMessage(const std::size_t& msg_id) const throw(ElementNotFound)
{
	return this->messages.find(msg_id)->second;
}

std::vector<std::size_t> Chat::getMessagesIds() const
{
	std::vector<std::size_t> ids;

	for(auto it = this->messages.begin(); it != this->messages.end(); it++)
		ids.push_back(it->first);

	return ids;
}
