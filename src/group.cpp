#include "group.h"

Group::Group()
{;}

Group::Group(const std::string& name): name(name)
{;}

bool Group::addUser(const std::string& user_name)
{
	if(this->has(user_name))
		return false;

	this->users_names.push_back(user_name);
	return true;
}

bool Group::delUser(const std::string& user_name)
{
	for(unsigned i=0; i<this->users_names.size(); i++)
		if(this->users_names[i] == user_name)
		{
			this->users_names.erase(this->users_names.begin() + i);
			return true;
		}

	return false;
}

bool Group::has(const std::string& user_name) const
{
	for(unsigned i=0; i<this->users_names.size(); i++)
		if(this->users_names[i] == user_name)
			return true;

	return false;
}

std::string Group::getName() const
{
	return this->name;
}

std::vector<std::string> Group::getUsersNames() const
{
	return this->users_names;
}
