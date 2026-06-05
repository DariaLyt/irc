#include "Server.hpp"

void Server::handlePart(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (!client.isRegistered())
	{
		sendNumeric(client, "451", " :You have not registered");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, "461", "PART: Not enough parameters");
		return;
	}

	std::string channelName = params[0];
	std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
	if (it == _channels.end())
	{
		sendNumeric(client, "403", channelName + " :No such channel");
		return;
	}
	Channel *channel = it->second;
	if (!channel->hasMember(client.getFd()))
	{
		sendNumeric(client, "442", channelName + " :You're not on that channel");
		return;
	}

	std::string partMsg = ":" + client.getNickname() + "!" + client.getUsername() + " PART " + channelName +  "\r\n";
	channel->broadcast(partMsg);

	channel->removeMember(client.getFd());
	channel->removeOperator(client.getFd());
	if (channel->isEmpty())
	{
		delete channel;
		_channels.erase(it);
	}
}