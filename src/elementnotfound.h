#include <string>

class ElementNotFound
{
	private:
	std::string msg;

	public:
	ElementNotFound(const std::string& msg);
	std::string getMessage() const;
};
