#include "chatcontroller.h"

/*
ChatController::ChatController(Chat& model, ChatView& view): 
	model(model), view(view)
{;}

int ChatController::addGroup(const std::string& group_name)
{
	return this->model.addGroup(group_name)?SUCCESS:GROUP_EXISTS;
}

int ChatController::delGroup(const std::string& group_name)
{
	return this->model.delGroup(group_name)?SUCCESS:GROUP_NOT_FOUND;
}

int ChatController::addUser(const User& user)
{
	return this->model.addUser(user)?SUCCESS:USER_EXISTS;
}

int ChatController::delUser(const std::string& user_name)
{
	return this->model.delUser(user_name)?SUCCESS:USER_NOT_FOUND;
}

int ChatController::queueMessage(const Message& msg)
{
	if(!this->model.hasUser(msg.getSrcUserName()))
		return USER_NOT_FOUND;
	if(!this->model.hasUser(msg.getDstUserName()))
		return USER_NOT_FOUND;
	if(!this->model.addMessage(msg))
		return MESSAGE_EXISTS;

	return SUCCESS;
}
*/
