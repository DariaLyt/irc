#pragma once

#include <sys/socket.h> // for sockets
#include <iostream>
#include <sys/types.h> // for ssockets
#include <vector>
#include <arpa/inet.h> // for inet_ntoa()
#include <csignal>
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h> // for sockaddr_in
#include <fcntl.h>

class Server
{
	private:
		int _port;
		const std::string _password;
		int _serverSockerFd;

	public:
		Server() = delete;
		~Server();
};