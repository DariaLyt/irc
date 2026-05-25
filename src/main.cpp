#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "Error! Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}

	std::cout << argv[1] << " " << argv[2] << std::endl;
	return (0);
}