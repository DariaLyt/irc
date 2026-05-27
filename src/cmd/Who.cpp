#include "Server.hpp"

void Server::handleWho(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	if (params.empty())
	{
		sendNumeric(client, "315", " *:End of /WHO list");
		return;
	}

	std::string target = params[0];
	if (_channels.find(target) != _channels.end())
	{
		Channel *channel = _channels[target];
		const std::map<int, Client *> &members = channel->getMembers();
		for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
		{
			Client *c = it->second;
			sendNumeric(client, "352", channel->getName() + " " + c->getUsername() + " ircserv " + c->getNickname() + " H :0 Unknown" );
		}
	}
	sendNumeric(client, "315", target + " :End of /WHO list");
}