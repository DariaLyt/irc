#include "Server.hpp"

void Server::handlePrivmsg(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	if (params.size() < 2)
		return ;
	std::string target = params[0];
	std::string text = params[1];
	if (target[0] == '#')
	{
		if (_channels.find(target) != _channels.end())
		{
			std::string msg = ":" + client.getNickname() + " PRIVMSG " + target + " :" + text + "\r\n";
			_channels[target]->broadcast(msg, client.getFd()); // we dont send the message to sender - maybe remove this
		}
	}
}