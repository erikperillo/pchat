#ifndef __CHATHASH_HEADER__
#define __CHATHASH_HEADER__

#include <iostream>
#include <iomanip>
#include <functional>
#include <string>
#include <unordered_set>
#include "message.h"

namespace std
{
	template<> struct hash<Message>
	{
		std::size_t operator()(const Message& msg) const;
	};
}

#endif
