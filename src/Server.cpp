#include "Server.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
extern bool g_serverRunning;

Server::Server(int port, const std::string &password)
	: _port(port),
	  _password(password),
	  _listenFd(-1),
	  _pollFds(),
	  _clients(),
	  _nicknames()
{
}

Server::~Server()
{
	for (std::map<int, Client *>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	if (_listenFd >= 0)
		close(_listenFd);
}

void Server::init()
{
	createListeningSocket();
}

void Server::run()
{
	while (g_serverRunning)
	{
		rebuildPollFds();
		int ready = poll(&_pollFds[0], _pollFds.size(), -1);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
		}
		if (ready == 0)
			continue;
		if ((_pollFds[0].revents & POLLIN) != 0)
			acceptClient();
		for (std::size_t i = 1; i < _pollFds.size(); ++i)
		{
			int fd = _pollFds[i].fd;
			short events = _pollFds[i].revents;

			if (events == 0)
				continue;
			if ((events & (POLLERR | POLLHUP | POLLNVAL)) != 0)
			{
				disconnectClient(fd, "poll error");
				continue;
			}
			if ((events & POLLIN) != 0 && _clients.find(fd) != _clients.end())
				readClient(fd);
			if ((events & POLLOUT) != 0 && _clients.find(fd) != _clients.end())
				writeClient(fd);
		}
	}
}

void Server::createListeningSocket()
{
	struct addrinfo hints;
	struct addrinfo *result;
	std::string port = intToString(_port);

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;  // IPv4 only  (AF_UNSPEC) - to use IPv6 also
	hints.ai_socktype = SOCK_STREAM; // TCP socket
	hints.ai_flags = AI_PASSIVE; // setting address to be suitable for bind() AI_PASSIVE 0.0.0.0
	int status = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (status != 0)
		throw std::runtime_error(std::string("getaddrinfo: ")
			+ gai_strerror(status));

	for (struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next)
	{
		_listenFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (_listenFd < 0)
			continue;
		int yes = 1;
		if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, // SOL_SOCKET - socket-level option, SO_REUSEADDR - immediately reuse port
				sizeof(yes)) < 0)
		{
			close(_listenFd);
			_listenFd = -1;
			continue;
		}
		setNonBlocking(_listenFd);
		if (bind(_listenFd, rp->ai_addr, rp->ai_addrlen) == 0
			&& listen(_listenFd, SOMAXCONN) == 0) // SOMAXCONN - max pending connction queue
			break;
		close(_listenFd);
		_listenFd = -1;
	}
	freeaddrinfo(result);
	if (_listenFd < 0)
		throw std::runtime_error("failed to bind listening socket");
	std::cout << "ircserv listening on port " << _port << std::endl;
}

void Server::rebuildPollFds()
{
	_pollFds.clear();

	struct pollfd listenPoll;
	listenPoll.fd = _listenFd;
	listenPoll.events = POLLIN;
	listenPoll.revents = 0;
	_pollFds.push_back(listenPoll);

	for (std::map<int, Client *>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		struct pollfd clientPoll;
		clientPoll.fd = it->first;
		clientPoll.events = POLLIN;
		if (it->second->hasOutput())
			clientPoll.events |= POLLOUT;
		clientPoll.revents = 0;
		_pollFds.push_back(clientPoll);
	}
}

void Server::acceptClient()
{
	struct sockaddr_storage address;
	socklen_t addressLength = sizeof(address);
	int fd = accept(_listenFd, reinterpret_cast<struct sockaddr *>(&address),
			&addressLength);

	if (fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "accept: " << std::strerror(errno) << std::endl;
		return;
	}
	setNonBlocking(fd);
	_clients[fd] = new Client(fd);
	std::cout << "client connected on fd " << fd << std::endl;
}

void Server::readClient(int fd)
{
	char buffer[4096];
	ssize_t received = recv(fd, buffer, sizeof(buffer), 0);

	if (received <= 0)
	{
		if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
			return;
		disconnectClient(fd, "client closed connection");
		return;
	}
	// this is to fix invalid read after QUIT
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;
	Client *client = it->second;
	client->appendInput(buffer, static_cast<std::size_t>(received));
	while(_clients.find(fd) != _clients.end() && client->hasCompleteLine())
		handleLine(*client, client->popLine());
	// 
	// Client &client = *_clients[fd];
	// client.appendInput(buffer, static_cast<std::size_t>(received));
	// while (client.hasCompleteLine() && _clients.find(fd) != _clients.end())
	// 	handleLine(client, client.popLine());
}

void Server::writeClient(int fd)
{
	Client &client = *_clients[fd];
	const std::string &output = client.getOutput();
	ssize_t sent = send(fd, output.c_str(), output.size(), 0);

	if (sent < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		disconnectClient(fd, "send failed");
		return;
	}
	if (sent == 0)
		return;
	client.consumeOutput(static_cast<std::size_t>(sent));
}

void Server::disconnectClient(int fd, const std::string &reason)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;

	std::string nick = it->second->getNickname();
	std::string user = it->second->getUsername();
	std::string quitMsg = ":" + nick + "!" + user + " QUIT :" + reason + "\r\n";
	std::vector<Client *> alerted;

	// added this part
	std::map<std::string, Channel *>::iterator cit = _channels.begin();
	while (cit != _channels.end())
	{
		Channel *channel = cit->second;
		// changed if statement, to fix double quit alert message
		if (channel->hasMember(fd))
		{
			const std::map<int, Client *> &members = channel->getMembers();
			for(std::map<int, Client *>::const_iterator mit = members.begin(); mit != members.end(); ++mit)
			{
				Client *peer = mit->second;
				if (peer->getFd() == fd)
					continue;
				bool alreadyAlerted = false;
				for(std::size_t i = 0; i < alerted.size(); ++i)
				{
					if (alerted[i] == peer)
					{
						alreadyAlerted = true;
						break;
					}
				} 

				if (!alreadyAlerted)
				{
					peer->queueOutput(quitMsg);
					alerted.push_back(peer);
				}
			}
			channel->removeMember(fd);
			channel->removeOperator(fd);
			if (channel->isEmpty())
			{
				delete channel;
				_channels.erase(cit++);
				continue;
			}
		}
		else
			++cit;
		// 	std::string nick = it->second->getNickname();
		// 	channel->broadcast(":" + nick + "!" + it->second->getUsername() + "PART");
		// 	channel->removeMember(fd);
		// 	if (channel->isEmpty())
		// 	{
		// 		delete channel;
		// 		_channels.erase(cit++);
		// 		continue;
		// 	}
		// }
		// else
		// 	++cit;
	}

	// if (!it->second->getNickname().empty())
	// 	_nicknames.erase(it->second->getNickname());
	// std::cout << "client fd " << fd << " disconnected: " << reason << std::endl;
	if (!nick.empty())
		_nicknames.erase(nick);
	std::cout << "client fd" << fd << " disconnected: " << reason <<std::endl;
	close(fd);
	delete it->second;
	_clients.erase(it);
}

void Server::handleLine(Client &client, const std::string &line)
{
	if (line.empty())
		return;
	handleCommand(client, Message(line));
}

void Server::handleCommand(Client &client, const Message &message)
{
	const std::string &command = message.getCommand();

	if (command == "CAP")
		handleCap(client, message);
	else if (command == "PASS")
		handlePass(client, message);
	else if (command == "NICK")
		handleNick(client, message);
	else if (command == "USER")
		handleUser(client, message);
	else if (command == "PING")
		handlePing(client, message);
	else if (command == "JOIN") // added this
		handleJoin(client, message);
	else if (command == "WHO") // added this
		handleWho(client, message);
	else if (command == "PRIVMSG") // added this
		handlePrivmsg(client, message);
	else if (command == "PART")
		handlePart(client, message);
	else if (command == "KICK")
		handleKick(client, message);
	else if (command == "TOPIC")
		handleTopic(client, message);
	else if (command == "MODE")
		handleMode(client, message);
	else if (command == "INVITE")
		handleInvite(client, message);
	else if (command == "QUIT")
		disconnectClient(client.getFd(), "QUIT");
	else if (!client.isRegistered())
		sendNumeric(client, "451", ":You have not registered");
	else
		sendNumeric(client, "421", command + " :Unknown command");
}

void Server::handleCap(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (!params.empty() && params[0] == "LS")
		sendReply(client, ":ircserv CAP " + clientTarget(client) + " LS :\r\n");
}

void Server::handlePass(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (client.isRegistered())
	{
		sendNumeric(client, "462", ":You may not reregister");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, "461", "PASS :Not enough parameters");
		return;
	}
	if (params[0] != _password)
	{
		sendNumeric(client, "464", ":Password incorrect");
		client.queueOutput("ERROR :Password incorrect\r\n");
		return;
	}
	client.setPassOk(true);
	maybeRegister(client);
}

void Server::handleUser(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (client.isRegistered())
	{
		sendNumeric(client, "462", ":You may not reregister");
		return;
	}
	if (params.size() < 4)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return;
	}
	client.setUsername(params[0]);
	client.setRealname(params[3]);
	maybeRegister(client);
}

void Server::handlePing(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
		sendNumeric(client, "409", ":No origin specified");
	else
		sendReply(client, ":ircserv PONG ircserv :" + params[0] + "\r\n");
}

void Server::maybeRegister(Client &client)
{
	if (client.isRegistered())
		return;
	if (!client.hasPass() || client.getNickname().empty()
		|| client.getUsername().empty())
		return;
	client.setRegistered(true);
	sendNumeric(client, "001", ":Welcome to ircserv, " + client.getNickname());
}

bool Server::nicknameInUse(const std::string &nickname, int requesterFd) const
{
	std::map<std::string, int>::const_iterator it = _nicknames.find(nickname);

	return it != _nicknames.end() && it->second != requesterFd;
}

std::string Server::clientTarget(const Client &client) const
{
	if (!client.getNickname().empty())
		return client.getNickname();
	return "*";
}

void Server::sendReply(Client &client, const std::string &message)
{
	client.queueOutput(message);
}

void Server::sendNumeric(Client &client, const std::string &code,
	const std::string &text)
{
	client.queueOutput(":ircserv " + code + " " + clientTarget(client)
		+ " " + text + "\r\n");
}

void Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error(std::string("fcntl: ") + std::strerror(errno));
}

std::string Server::intToString(int value)
{
	std::ostringstream stream;

	stream << value;
	return stream.str();
}

bool Server::isValidNickname(const std::string &nickname)
{
	if (nickname.empty())
		return false;
	for (std::string::size_type i = 0; i < nickname.size(); ++i)
	{
		char c = nickname[i];
		bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9') || c == '_' || c == '-'
			|| c == '[' || c == ']' || c == '\\' || c == '`'
			|| c == '^' || c == '{' || c == '}';
		if (!ok)
			return false;
	}
	return true;
}
