#ifndef __USER_HEADER__
#define __USER_HEADER__

#include <string>
#include "net.h"

class User
{
	private:
	std::string name;
	NetAddr addr;

	public:
	User();
	User(const std::string& name, const NetAddr& addr);
	std::string getName() const;
	NetAddr getAddr() const;
};

#endif
