#include "Client.hpp"

Client::Client(int fd)
	: _fd(fd),
	  _passOk(false),
	  _registered(false),
	  _nickname(),
	  _username(),
	  _realname(),
	  _input(),
	  _output()
{
}

int Client::getFd() const
{
	return _fd;
}

bool Client::hasPass() const
{
	return _passOk;
}

bool Client::isRegistered() const
{
	return _registered;
}

const std::string &Client::getNickname() const
{
	return _nickname;
}

const std::string &Client::getUsername() const
{
	return _username;
}

void Client::setPassOk(bool value)
{
	_passOk = value;
}

void Client::setRegistered(bool value)
{
	_registered = value;
}

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setRealname(const std::string &realname)
{
	_realname = realname;
}

void Client::appendInput(const char *data, std::size_t length)
{
	_input.append(data, length);
}

bool Client::hasCompleteLine() const
{
	return _input.find('\n') != std::string::npos;
}

std::string Client::popLine()
{
	std::string::size_type end = _input.find('\n');
	std::string line = _input.substr(0, end);

	_input.erase(0, end + 1);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	return line;
}

void Client::queueOutput(const std::string &message)
{
	_output += message;
}

bool Client::hasOutput() const
{
	return !_output.empty();
}

const std::string &Client::getOutput() const
{
	return _output;
}

void Client::consumeOutput(std::size_t length)
{
	_output.erase(0, length);
}
