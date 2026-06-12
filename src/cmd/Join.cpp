#include "Server.hpp"
#include <sstream>

static std::vector<std::string> splitByComma(const std::string &str)
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, ','))
	{
		tokens.push_back(token);
	}
	if (tokens.empty() && !str.empty())
		tokens.push_back(str);
	return tokens;
}

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

	std::vector<std::string> channelsToJoin = splitByComma(params[0]);
	std::vector<std::string> keys;
	if (params.size() > 1)
		keys = splitByComma(params[1]);

	for (std::size_t i = 0; i < channelsToJoin.size(); ++i)
	{
		std::string channelName = channelsToJoin[i];
		if (channelName.empty())
			continue;
		if (channelName[0] != '#' && channelName[0] != '&')
		{
			sendNumeric(client, "403", channelName + " :No such channel");
			continue;
		}

		std::string providedKey = "";
		if (i < keys.size())
			providedKey = keys[i];

		bool isCreator = false;
		if (_channels.find(channelName) == _channels.end())
		{
			_channels[channelName] = new Channel(channelName);
			isCreator = true;
		}

		Channel *channel = _channels[channelName];

		if (!isCreator)
		{
			if (channel->getMaxUsers() > 0 && channel->getMembers().size() >= channel->getMaxUsers())
			{
				sendNumeric(client, "471", channelName + " :Cannot join channel (+l)");
				continue;
			}
			if (!channel->getKey().empty())
			{
				if (providedKey != channel->getKey())
				{
					sendNumeric(client, "475", channelName + " :Cannot join channel (+k)");
					continue;
				}
			}
			if (channel->isInviteOnly())
			{
				if (!channel->isInvited(client.getFd()))
				{
					sendNumeric(client, "473", channelName + " :Cannot join channel (+i)");
					continue;
				}
				channel->removeInvite(client.getFd());
			}
		}
		if (channel->hasMember(client.getFd()))
			continue;

		channel->addMember(&client);
		if (isCreator)
			channel->addOperator(client.getFd());

		std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN " + channelName + "\r\n";
		channel->broadcast(joinMsg);

		if (!channel->getTopic().empty())
			sendNumeric(client, "332", channelName + " :" + channel->getTopic());
		else
			sendNumeric(client, "332", channelName + " :Welcome to " + channelName);
		std::string userList = "";
		const std::map<int, Client*> &members = channel->getMembers();
		for (std::map<int, Client*>::const_iterator mit = members.begin(); mit != members.end(); ++mit)
		{
			if (!userList.empty())
				userList += " ";
			if (channel->isOperator(mit->first))
				userList += "@";
			userList += mit->second->getNickname();
		}
		sendNumeric(client, "353", "= " + channelName + " :" + userList);
		sendNumeric(client, "366", channelName + " :End of /NAMES list");
	}
	// std::string channelName = params[0];
	// if (channelName.empty() || channelName[0] != '#')
	// {
	// 	sendNumeric(client, "403", channelName + " :No such channel");
	// 	return ;
	// }

	// bool isCreator = false;
	// if (_channels.find(channelName) == _channels.end())
	// { // creating the channel if it doesnt exist
	// 	_channels[channelName] = new Channel(channelName);
	// 	isCreator = true;
	// }
	
	// Channel *channel = _channels[channelName];
	// if (!isCreator)
	// {
	// 	if (channel->getMaxUsers() > 0 && channel->getMembers().size() >= channel->getMaxUsers())
	// 	{
	// 		sendNumeric(client, "471", channelName + " :Cannot join channel");
	// 		return;
	// 	}
	// 	if (!channel->getKey().empty())
	// 	{
	// 		std::string providedKey = (params.size() > 1) ? params[1] : "";
	// 		if (providedKey != channel->getKey())
	// 		{
	// 			sendNumeric(client, "475", channelName + " :Cannot join channel");
	// 			return;
	// 		}
	// 	}
	// 	if (channel->isInviteOnly())
	// 	{
	// 		if (!channel->isInvited(client.getFd()))
	// 		{
	// 			sendNumeric(client, "473", channelName + " :Cannot join channel");
	// 			return;
	// 		}
	// 		channel->removeInvite(client.getFd());
	// 	}
	// }
	// if (channel->hasMember(client.getFd())) // check if client is already in channel
	// 	return ;

	// channel->addMember(&client);
	// if (isCreator)
    //     channel->addOperator(client.getFd());
	// std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN " + channelName + "\r\n";
	// channel->broadcast(joinMsg);
	// if (!channel->getTopic().empty())
	// 	sendNumeric(client, "332", channelName + " :" + channel->getTopic());
	// else
	// 	sendNumeric(client, "332", channelName + " :Welcome to " + channelName);
	// std::string userList = "";
	// const std::map<int, Client*> &members = channel->getMembers();
	// for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
	// {
	// 	if (!userList.empty())
	// 		userList += " ";
	// 	if (channel->isOperator(it->first))
	// 		userList += "@";
	// 	userList += it->second->getNickname();	
	// }
	// sendNumeric(client, "353", "= "  + channelName + " :" + userList);
	// sendNumeric(client, "366", channelName + " :End of /NAMES list");
}

