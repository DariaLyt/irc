#include "Server.hpp"

void Server::handleJoin(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	if (!client.isRegistered())
	{
		sendNumeric(client, "451", " :You have not registered");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, "461", "JOIN : Not enough parameters");
		return ;
	}

	std::string channelName = params[0];
	if (channelName.empty() || channelName[0] != '#')
	{
		sendNumeric(client, "403", channelName + " :No such channel");
		return ;
	}

	bool isCreator = false;
	if (_channels.find(channelName) == _channels.end())
	{ // creating the channel if it doesnt exist
		_channels[channelName] = new Channel(channelName);
		isCreator = true;
	}
	
	Channel *channel = _channels[channelName];
	if (!isCreator)
	{
		if (channel->getMaxUsers() > 0 && channel->getMembers().size() >= channel->getMaxUsers())
		{
			sendNumeric(client, "471", channelName + " :Cannot join channel");
			return;
		}
		if (!channel->getKey().empty())
		{
			std::string providedKey = (params.size() > 1) ? params[1] : "";
			if (providedKey != channel->getKey())
			{
				sendNumeric(client, "475", channelName + " :Cannot join channel");
				return;
			}
		}
		if (channel->isInviteOnly())
		{
			if (!channel->isInvited(client.getFd()))
			{
				sendNumeric(client, "473", channelName + " :Cannot join channel");
				return;
			}
			channel->removeInvite(client.getFd());
		}
	}
	if (channel->hasMember(client.getFd())) // check if client is already in channel
		return ;

	channel->addMember(&client);
	if (isCreator)
        channel->addOperator(client.getFd());
	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "JOIN " + channelName + "\r\n";
	channel->broadcast(joinMsg);

	if (!channel->getTopic().empty())
		sendNumeric(client, "332", channelName + " :" + channel->getTopic());
	else
		sendNumeric(client, "332", channelName + " :Welcome to " + channelName);
	std::string userList = "";
	const std::map<int, Client*> &members = channel->getMembers();
	for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (!userList.empty())
			userList += " ";
		if (channel->isOperator(it->first))
			userList += "@";
		userList += it->second->getNickname();	
	}
	sendNumeric(client, "353", "= "  + channelName + " :" + userList);
	sendNumeric(client, "366", channelName + " :End of /NAMES list");
}

