#include "elementnotfound.h"

ElementNotFound::ElementNotFound(const std::string& msg): msg(msg)
{;}

std::string ElementNotFound::getMessage() const
{
	return this->msg;
}

