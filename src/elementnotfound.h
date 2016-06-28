#ifndef __ELEMENTNOTFOUND_HEADER__
#define __ELEMENTNOTFOUND_HEADER__

#include <string>

/*
Exception thrown when some element is not found in some search.
*/
class ElementNotFound
{
	private:
	std::string msg;

	public:
	ElementNotFound(const std::string& msg);
	std::string getMessage() const;
};

#endif
