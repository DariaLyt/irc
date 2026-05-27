#ifndef SERVER_HPP
# define SERVER_HPP

# include "Client.hpp"
# include "Message.hpp"
# include "Channel.hpp"

# include <map>
# include <poll.h>
# include <string>
# include <vector>

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();

		void init();
		void run();

	private:
		int _port;
		std::string _password;
		int _listenFd;
		std::vector<struct pollfd> _pollFds;
		std::map<int, Client *> _clients;
		std::map<std::string, int> _nicknames;
		std::map<std::string, Channel *> _channels;

		Server();
		Server(const Server &other);
		Server &operator=(const Server &other);

		void createListeningSocket();
		void rebuildPollFds();
		void acceptClient();
		void readClient(int fd);
		void writeClient(int fd);
		void disconnectClient(int fd, const std::string &reason);

		void handleLine(Client &client, const std::string &line);
		void handleCommand(Client &client, const Message &message);
		void handleCap(Client &client, const Message &message);
		void handlePass(Client &client, const Message &message);
		void handleNick(Client &client, const Message &message);
		void handleUser(Client &client, const Message &message);
		void handlePing(Client &client, const Message &message);

		void handleJoin(Client &client, const Message &message);
		void handleWho(Client &client, const Message &message);
		void handlePrivmsg(Client &client, const Message &message);

		void maybeRegister(Client &client);
		bool nicknameInUse(const std::string &nickname, int requesterFd) const;
		std::string clientTarget(const Client &client) const;

		void sendReply(Client &client, const std::string &message);
		void sendNumeric(Client &client, const std::string &code,
			const std::string &text);

		static void setNonBlocking(int fd);
		static std::string intToString(int value);
		static bool isValidNickname(const std::string &nickname);
};

#endif


/*
Resolve address
    ↓
Create socket
    ↓
Configure socket
    ↓
Bind to port
    ↓
Start listening
*/