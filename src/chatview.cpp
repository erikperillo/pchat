#include "chatview.h"
#include <iomanip>

#define max(a, b) (((a)>(b))?(a):(b))

ChatView::ChatView(Chat& model, std::ostream& out): 
	col_w(0), model(model), out(out)
{;}

void ChatView::setColWidth(unsigned width)
{
	this->col_w = width;
}

void ChatView::setColWidth(const std::vector<std::string>& words)
{
	for(auto const& w: words)
		if(w.size()+2 > this->col_w)
			this->col_w = w.size()+2;
}

void ChatView::setColWidth()
{
	this->col_w = 0;
	this->setColWidth(this->model.getUsersNames());
	this->setColWidth(this->model.getGroupsNames());
}

std::string ChatView::center(const std::string& word)
{
	unsigned num_spaces;
	std::string spaces;

	num_spaces = (this->col_w - word.size())/2;
	for(unsigned i=0; i<num_spaces; i++)
		spaces += " ";

	return spaces + word + spaces + (((this->col_w - word.size())%2!=0)?" ":"");
}

std::string ChatView::col(const std::string& word)
{
	return "|" + this->center(word) + "|";
}

std::string ChatView::sep(unsigned cols)
{
	return std::string(cols*(this->col_w + 2), '-');
}

void ChatView::printTable(const std::vector<std::vector<std::string>>& cols,
	const std::vector<std::string>& header)
{
	//checking if all cols have the same size
	for(unsigned i=1; i<cols.size(); i++)
		if(cols[i].size() != cols[i-1].size())
			return;

	//getting collumn width
	this->resetColWidth();
	//for(auto c: cols)
	//	this->setColWidth(c);
	this->setColWidth();
	this->setColWidth(header);

	//header
	if(header.size() > 0 && !header[0].empty())
	{
		this->out << this->sep(cols.size()) << std::endl;
		for(auto const& c: header)
			this->out << this->col(c);
		this->out << std::endl;
	}
	//collumns
	this->out << this->sep(cols.size()) << std::endl;
	for(unsigned i=0; i<cols[0].size(); i++)
	{
		for(auto c: cols)
			this->out << this->col(c[i]);
		this->out << std::endl;
	}
	this->out << this->sep(cols.size()) << std::endl;
}

void ChatView::printTable(const std::vector<std::string>& rows,
	const std::string header)
{
	std::vector<std::vector<std::string>> cols(1, rows);
	std::vector<std::string> header_vec(1, header);
	this->printTable(cols, header_vec);
}

void ChatView::printUsers(const std::string& title)
{
	this->printTable(this->model.getUsersNames(), title);
}

void ChatView::printGroups(const std::string& title)
{
	this->printTable(this->model.getGroupsNames(), title);
}

bool ChatView::printUsersFromGroup(const std::string& group_name, 
	std::vector<std::string> header)
{
	if(!this->model.hasGroup(group_name))
		return false;

	//formatting
	Group group = this->model.getGroup(group_name);
	//creating users col
	std::vector<std::string> users_names = group.getUsersNames();
	//creating group col
	std::vector<std::string> groups_names(users_names.size(), group_name);
	//creating table
	std::vector<std::vector<std::string>> table;
	table.push_back(users_names);
	table.push_back(groups_names);
	//printing table
	if(users_names.size() > 0)
	{
		if(header.size() == 2)
			this->printTable(table, header);
		else
			this->printTable(table, std::vector<std::string>());
	}

	return true;
}

bool ChatView::printUsersFromGroup(const std::string& group_name, bool header)
{
	std::vector<std::string> hdr;

	if(header)
	{
		hdr.push_back("users");
		hdr.push_back("groups");
	}

	return this->printUsersFromGroup(group_name, hdr);
}

void ChatView::resetColWidth()
{
	this->setColWidth(0);
}
