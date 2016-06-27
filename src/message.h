#ifndef __MESSAGE_HEADER__
#define __MESSAGE_HEADER__

#include <string>

class Message
{
	private:
	std::string src_user_name;
	std::string dst_user_name;
	std::string content;
	std::string title;

	public:
	Message();
	Message(const Message& msg);
	Message(const std::string src_user_name, const std::string dst_user_name,
		const std::string& content, const std::string& title=std::string());
	std::string getContent() const;
	std::string getSrcUserName() const;
	std::string getDstUserName() const;
	std::string getTitle() const;
	bool hasTitle() const;
};

#endif
