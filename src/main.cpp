#include "Server.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

static bool isDigits(const char *value)
{
	if (value == NULL || *value == '\0')
		return false;
	for (int i = 0; value[i] != '\0'; ++i)
	{
		if (value[i] < '0' || value[i] > '9')
			return false;
	}
	return true;
}

static int parsePort(const char *value)
{
	if (!isDigits(value))
		throw std::runtime_error("port must be a number between 1 and 65535");
	long port = std::strtol(value, NULL, 10);
	if (port < 1024 || port > 65535)
		throw std::runtime_error("port must be a number between 1 and 65535");
	return static_cast<int>(port);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	try
	{
		int port = parsePort(argv[1]);
		std::string password = argv[2];

		if (password.empty())
			throw std::runtime_error("password must not be empty");
		Server server(port, password);
		server.init();
		server.run();
	}
	catch (const std::exception &error)
	{
		std::cerr << "ircserv: " << error.what() << std::endl;
		return 1;
	}
	return 0;
}
