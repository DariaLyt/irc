#include "Server.hpp"

void Server::handlePrivmsg(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();
	if (!client.isRegistered())
	{
		sendNumeric(client, "451", " :You have not registered");
		return;
	}
	if (params.empty())
	{
		sendNumeric(client, "411", " :No recipient given");
		return ;
	}

	if (params.size() < 2 || params[1].empty())
	{
		sendNumeric(client, "412", " :No text given");
		return;
	}
	std::string target = params[0];
	std::string text = params[1];
	std::string senderPrefix = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost";
	std::string finalMsg = senderPrefix + " PRIVMSG " + target + " :" + text + "\r\n";
	if (target[0] == '#')
	{
		std::map<std::string, Channel *>::iterator chanIt = _channels.find(target);
		if (chanIt == _channels.end())
		{
			sendNumeric(client, "403", target + " :No such channel");
			return;
		}

		Channel *channel = chanIt->second;
		if (!channel->hasMember(client.getFd()))
		{
			sendNumeric(client, "442", target + " :You're not on that channel");
			return;
		}
		channel->broadcast(finalMsg, client.getFd());
	}
	else
	{
		std::map<std::string, int>::iterator nickIt = _nicknames.find(target);
		if (nickIt == _nicknames.end())
		{
			sendNumeric(client, "401", target + " :No such nick/channel");
			return;
		}

		int targetFd = nickIt->second;
		std::map<int, Client *>::iterator clientIt = _clients.find(targetFd);
		if (clientIt != _clients.end())
			clientIt->second->queueOutput(finalMsg);
	}
	// if (target[0] == '#')
	// {
	// 	if (_channels.find(target) != _channels.end())
	// 	{
	// 		std::string msg = ":" + client.getNickname() + " PRIVMSG " + target + " :" + text + "\r\n";
	// 		_channels[target]->broadcast(msg, client.getFd()); 
	// 	}
	// }
}