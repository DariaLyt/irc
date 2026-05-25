#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include <string>
# include <vector>

class Message
{
	public:
		Message();
		explicit Message(const std::string &line);

		const std::string &getCommand() const;
		const std::vector<std::string> &getParams() const;

	private:
		std::string _command;
		std::vector<std::string> _params;

		static std::string toUpper(const std::string &value);
};

#endif
