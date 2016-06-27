#ifndef __GROUP_HEADER__
#define __GROUP_HEADER__

#include <string>
#include <vector>

class Group
{
	private:
	std::string name;
	std::vector<std::string> users_names;

	public:
	Group();
	Group(const std::string& name);
	bool addUser(const std::string& user_name);
	bool delUser(const std::string& user_name);
	bool has(const std::string& user_name) const;
	std::string getName() const;
	std::vector<std::string> getUsersNames() const;
};

#endif
