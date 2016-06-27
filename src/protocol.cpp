#include "protocol.h"

/*ProtocolFormatError::ProtocolFormatError(const std::string& msg): msg(msg)
{;}

std::string ProtocolFormatError::getMessage() const
{
	return this->msg;
}*/
/*
std::string lower(const std::string& str)
{
	std::string res(str);

	for(unsigned i=0; i<res.size(); i++)
		if(res[i] >= 'A' && res[i] <= 'Z')
			res[i] = res[i] - ('Z' - 'z');

      return res;
}*/

std::string sanitize(const std::string& str)
{
	std::string sane_str;
	sane_str = std::regex_replace(str, std::regex(FIELD_SEP), "\\fsep");
	return std::regex_replace(sane_str, std::regex(MSG_SEP), "\\msep");
}

std::string desanitize(const std::string& str)
{
	std::string insane_str;
	insane_str = std::regex_replace(str, std::regex("\\\\fsep"), FIELD_SEP);
	return std::regex_replace(insane_str, std::regex("\\\\msep"), MSG_SEP);
}

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> tokens;
	std::string token;
	unsigned i=0;

	while(i < str.size())
	{
		while(i < str.size() && str.compare(i, delim.size(), delim))
			token.push_back(str[i++]);	

		if(!token.empty())
			tokens.push_back(token);
		token.clear();
		i += delim.size();
	}

	return tokens;
}

std::string join(const std::vector<std::string>& tokens, 
	const std::string& delim, int pos)
{
	std::string str;

	for(unsigned i=pos; i<tokens.size()-1; i++)
		str += tokens[i] + delim;
	if(tokens.size() - pos > 0)
		str += tokens[tokens.size()-1];

	return str;
}

std::string initMsg(char message)
{
	std::string str;
	str.push_back(message);
	return str + FIELD_SEP;
}

void addField(std::string& str, const std::string& field)
{
	str += sanitize(field) + FIELD_SEP;
}

void endMsg(std::string& str)
{
	str += MSG_SEP;
}

std::string hostToNetHeader(char message)
{
	std::string str;

	str = initMsg(message);
	endMsg(str);

	return str;
}

char netToHostHeader(const std::string& str)
{
	std::vector<std::string> tokens;	

	tokens = split(str, FIELD_SEP);
	if(tokens.size() < 1)
		return UNDEFINED;

	switch(tokens[0].size())
	{
		case 1:
			return tokens[0][0];
		case 2:
			return tokens[0][1];
		default:
			return UNDEFINED;
	}
}

std::string hostToNetMsg(char message)
{
	return hostToNetHeader(message);
}

std::string hostToNetMsg(char message, std::vector<std::string> args)
{
	std::string msg;
	
	msg = initMsg(message);
	for(auto const& arg: args)
		addField(msg, arg);
	endMsg(msg);

	return msg;
}

std::vector<std::string> netToHostMsg(std::string msg)
{
	std::vector<std::string> args;
	std::vector<std::string> tokens;
	
	tokens = split(msg, FIELD_SEP);
	for(unsigned i=1; i<tokens.size(); i++)
		args.push_back(desanitize(tokens[i]));

	return args;
}

std::string hostToNetRegister(const std::string& name)
{
	return hostToNetMsg(REGISTER, {name});
}

std::string netToHostRegister(const std::string& str)
{
	std::vector<std::string> args;

	args = netToHostMsg(str);

	return (args.size() < 1)?"":args[0];
}

std::string hostToNetSendMsg(const std::string& src_user_name, 
	const std::string& dst_user_name, const std::string& content)
{
	return hostToNetMsg(SEND_MSG, {src_user_name, dst_user_name, content});
}

std::string hostToNetSendMsg(const Message& msg)
{
	return hostToNetSendMsg(msg.getSrcUserName(), msg.getDstUserName(),
		msg.getContent());
}

Message netToHostSendMsg(const std::string& str)
{
	std::vector<std::string> args;

	args = netToHostMsg(str);
	if(args.size() < 3)
		return Message("", "", "");

	return Message(args[0], args[1], args[2]);
}

std::string hostToNetJoinGroup(const std::string& group_name)
{
	return hostToNetMsg(JOIN_GROUP, {group_name});
}

std::string netToHostJoinGroup(const std::string& str)
{
	std::vector<std::string> args;

	args = netToHostMsg(str);

	return (args.size() < 1)?"":desanitize(args[0]);
}

std::string hostToNetMsgQueued(std::size_t msg_id)
{
	return hostToNetMsg(MSG_QUEUED, {std::to_string(msg_id)});
}

std::size_t netToHostMsgQueued(const std::string msg, int to_cut)
{
	std::vector<std::string> args;
	size_t msg_id;
	
	args = netToHostMsg(msg);
	if(args.size() < 1)
		return 0;

	std::string str = args[0].substr(0, (to_cut >= 0)?to_cut:std::string::npos);
	std::stringstream ss(str);
	ss >> msg_id;

	return msg_id;
}

std::string hostToNetMsgSent(std::size_t msg_id)
{
	return hostToNetMsg(MSG_SENT, {std::to_string(msg_id)});
}

std::size_t netToHostMsgSent(const std::string msg, int to_cut)
{
	return netToHostMsgQueued(msg, to_cut);
}

std::string hostToNetFileQueued(std::size_t msg_id)
{
	return hostToNetMsg(FILE_QUEUED, {std::to_string(msg_id)});
}

std::size_t netToHostFileQueued(const std::string msg, int to_cut)
{
	return netToHostMsgQueued(msg, to_cut);
}

std::string hostToNetFileSent(std::size_t msg_id)
{
	return hostToNetMsg(FILE_SENT, {std::to_string(msg_id)});
}

std::size_t netToHostFileSent(const std::string msg, int to_cut)
{
	return netToHostMsgQueued(msg, to_cut);
}

std::string hostToNetMsgIncoming(const Message& msg)
{
	return hostToNetMsg(MSG_INCOMING, 
		{msg.getSrcUserName(), msg.getDstUserName(), msg.getContent()});
}

Message netToHostMsgIncoming(const std::string& msg)
{
	return netToHostSendMsg(msg);
}

std::string hostToNetCreateGroup(const std::string& name)
{
	return hostToNetMsg(CREATE_GROUP, {name});
}

std::string netToHostCreateGroup(const std::string& msg)
{
	std::vector<std::string> args;

	args = netToHostMsg(msg);
	if(args.size() < 1)
		return "";

	return args[0];
}

std::string hostToNetSendGroup(const std::string& group_name, 
	const std::string& msg)
{
	return hostToNetMsg(SEND_GROUP, {group_name, msg});
}

std::pair<std::string, std::string> netToHostSendGroup(const std::string& msg)
{
	std::vector<std::string> args;

	args = netToHostMsg(msg);
	if(args.size() < 2)
		return std::pair<std::string, std::string>("", "");

	return std::pair<std::string, std::string>(args[0], args[1]);
}

std::string fileToStr(const std::string& file_path)
{
	std::ifstream file(file_path);
	std::string content((std::istreambuf_iterator<char>(file)), 
		std::istreambuf_iterator<char>());

	return content;
}

std::string hostToNetSendFile(const std::string& src_user_name, 
	const std::string& dst_user_name, const std::string& file_path)
{
	std::string file_str; 
	file_str = fileToStr(file_path);
	if(file_str.empty())
		return "";

	return hostToNetMsg(SEND_FILE, {src_user_name, dst_user_name,
		file_str, file_path});
}

Message netToHostSendFile(const std::string& msg)
{
	std::vector<std::string> args;

	args = netToHostMsg(msg);
	if(args.size() < 4)
		return Message("", "", "");

	return Message(args[0], args[1], args[2], args[3]);
}

std::string hostToNetFileIncoming(const Message& msg)
{
	return hostToNetMsg(FILE_INCOMING, {msg.getSrcUserName(), 
		msg.getDstUserName(), msg.getContent(), msg.getTitle()});
}

Message netToHostFileIncoming(const std::string& msg)
{
	return netToHostSendFile(msg);
}

std::string hostToNetUsersList(const std::string& msg)
{
	return hostToNetMsg(USERS_LIST, {msg});
}

std::string netToHostUsersList(const std::string& msg)
{
	std::vector<std::string> args;

	args = netToHostMsg(msg);
	if(args.size() < 1)
		return "";

	return args[0];
}
