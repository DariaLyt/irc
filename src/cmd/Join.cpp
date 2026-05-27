#include "Server.hpp"

void Server::handleJoin(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
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

	if (_channels.find(channelName) == _channels.end()) // creating the channe if it doesnt exist
		_channels[channelName] = new Channel(channelName);

	Channel *channel = _channels[channelName];
	if (channel->hasMember(client.getFd())) // check if client is already in channel
		return ;

	channel->addMember(&client);
	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "JOIN " + channelName + "\r\n";
	channel->broadcast(joinMsg);

	sendNumeric(client, "332", channelName + " :Welcome to " + channelName);
	std::string userList = "";
	const std::map<int, Client*> &members = channel->getMembers();
	for (std::map<int , Client *>::const_iterator  it = members.begin(); it!=members.end(); ++it)
	{
		if (!userList.empty())
			userList += " ";
		userList += it->second->getNickname();	
	}
	sendNumeric(client, "353", "= "  + channelName + " :" + userList);
	sendNumeric(client, "366", channelName + " :End of /NAMES list");
}

