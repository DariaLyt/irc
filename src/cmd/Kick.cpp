#include "Server.hpp"

void Server::handleKick(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	if (!client.isRegistered())
	{
		sendNumeric(client, "451", " :You have not registered");
		return;
	}
	if (params.size() < 2)
	{
		sendNumeric(client, "461", " KICK: Not enough parameters");
		return;
	}
	std::string channelName = params[0];
	std::string targetNick = params[1];

	std::map<std::string, Channel *>::iterator chit = _channels.find(channelName);
	if (chit == _channels.end())
	{
		sendNumeric(client, "403", channelName + " :No such channel");
		return;
	}

	Channel *channel = chit->second;
	if (!channel->hasMember(client.getFd()))
	{
		sendNumeric(client, "442", channelName + " :You're not on that channel");
		return;
	}
	if (!channel->isOperator(client.getFd()))
	{
		sendNumeric(client, "482", channelName + " :You're not channel operator");
		return;
	}

	std::map<std::string, int>::iterator nit = _nicknames.find(targetNick);
	if (nit == _nicknames.end() || !channel->hasMember(nit->second))
	{
		sendNumeric(client, "441", targetNick + " " + channelName + " :They are not on that channel");
		return;
	}
	int targetFd = nit->second;
	std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + " KICK " + channelName + " " + targetNick + "\r\n";
    channel->broadcast(kickMsg);

    channel->removeMember(targetFd);
    channel->removeOperator(targetFd);
}