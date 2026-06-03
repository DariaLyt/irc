#include "Server.hpp"

void Server::handleInvite(Client &client, const Message &message)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, "451", " :You have not registered");
		return;
	}

	const std::vector<std::string> &params = message.getParams();
	if (params.size() < 2)
	{
		sendNumeric(client, "461", " INVITE: Not enough parameters");
		return;
	}
	std::string targetNick = params[0];
	std::string channelName = params[1];

	std::map<std::string, int>::iterator nickIt = _nicknames.find(targetNick);
	if (nickIt == _nicknames.end())
	{
		sendNumeric(client, "401", targetNick + " :No suck nickname");
		return;
	}
	int targetFd = nickIt->second;
	Client *targetClient = _clients[targetFd];

	std::map<std::string, Channel *>::iterator chanIt = _channels.find(channelName);
	if (chanIt == _channels.end())
	{
		sendNumeric(client, "403", " :No such channel");
		return;
	}
	Channel *channel = chanIt->second;
	if (!channel->isOperator(client.getFd()))
	{
		sendNumeric(client, "482", channelName+ " :You are not channel operator");
		return;
	}
	if (channel->hasMember(targetFd))
	{
		sendNumeric(client, "443", targetNick + " " + channelName + " :is already in the channel");
		return;
	}
	channel->addInvite(targetFd);
	sendNumeric(client, "341", targetNick + " " + channelName);
	std::string inviteNotice = ":" + client.getNickname() + "!" + client.getUsername() + " INVITE " + targetNick + " :" + channelName + "\r\n";
	targetClient->queueOutput(inviteNotice);
}