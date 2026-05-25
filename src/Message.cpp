#include "Message.hpp"

#include <cctype>

Message::Message()
	: _command(), _params()
{
}

Message::Message(const std::string &line)
	: _command(), _params()
{
	std::string::size_type i = 0;

	while (i < line.size() && line[i] == ' ')
		++i;
	if (i < line.size() && line[i] == ':')
	{
		while (i < line.size() && line[i] != ' ')
			++i;
		while (i < line.size() && line[i] == ' ')
			++i;
	}
	std::string::size_type commandStart = i;
	while (i < line.size() && line[i] != ' ')
		++i;
	_command = toUpper(line.substr(commandStart, i - commandStart));
	while (i < line.size())
	{
		while (i < line.size() && line[i] == ' ')
			++i;
		if (i >= line.size())
			break;
		if (line[i] == ':')
		{
			_params.push_back(line.substr(i + 1));
			break;
		}
		std::string::size_type paramStart = i;
		while (i < line.size() && line[i] != ' ')
			++i;
		_params.push_back(line.substr(paramStart, i - paramStart));
	}
}

const std::string &Message::getCommand() const
{
	return _command;
}

const std::vector<std::string> &Message::getParams() const
{
	return _params;
}

std::string Message::toUpper(const std::string &value)
{
	std::string result = value;

	for (std::string::size_type i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(
			std::toupper(static_cast<unsigned char>(result[i])));
	return result;
}
