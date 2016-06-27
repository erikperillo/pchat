#include "user.h"

User::User(const std::string& name, const NetAddr& addr): name(name), addr(addr)
{;}

User::User()
{;}

std::string User::getName() const
{
	return this->name;
}

NetAddr User::getAddr() const
{
	return this->addr;
}
