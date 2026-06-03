#include "Server.hpp"

void Server::handleMode(Client &client, const Message &message)
{
	if (!client.isRegistered())
	{
		sendNumeric(client, "451", " :You have not registered");
		return;
	}

	const std::vector<std::string> &params = message.getParams();
	if (params.empty())
	{
		sendNumeric(client, "461", " MODE: Not enough parameters");
		return;
	}

	std::string target = params[0];
	if (target[0] != '#')
		return;
	
	std::map<std::string, Channel *>::iterator chanIt = _channels.find(target);
	if (chanIt == _channels.end())
	{
		sendNumeric(client, "403", target + " :No such channel");
		return;
	}

	Channel *channel = chanIt->second;
	if (params.size() < 2) // no modes -> return current
	{
		std::string modes = "+";
		std::string modeParams = "";
		if (channel->isInviteOnly())
			modes += "i";
		if (channel->isTopicRestricted())
			modes += "t";
		if (channel->getKey().empty())
		{
			modes += "k";
			modeParams += " " + channel->getKey();
		}
		if (channel->getMaxUsers() > 0)
		{
			modes += "l";
			modeParams += " " + intToString(channel->getMaxUsers());
		}
		sendNumeric(client, "324", channel->getName() + " " + modes + modeParams);
		return;
	}	
	if (!channel->isOperator(client.getFd()))
	{
		sendNumeric(client, "482", channel->getName() + " :You are not channel operator");
		return;
	}

	std::string modeString =params[1];
	std::size_t paramIndex = 2;
	bool adding = true; 
	std::string appliedModes = "";
	std::string appliedParams = "";

	for (std::size_t i = 0; i < modeString.size(); ++i)
	{
		char mode = modeString[i];
		if (mode == '+')
		{
			adding = true;
			continue;
		}
		if (mode == '-')
		{
			adding = false;
			continue;
		}
		if (mode == 'i')
		{
			if (adding != channel->isInviteOnly())
			{
				channel->setInviteOnly(adding);
				appliedModes += (adding ? "+i" : "-i");
			}
		}
		else if (mode == 't')
		{
			if (adding != channel->isTopicRestricted())
			{
				channel->setTopicRestricted(adding);
				appliedModes += (adding ? "+t" : "-t");
			}
		}
		else if (mode == 'k')
		{
			if (adding) {
				if (paramIndex < params.size())
				{
					std::string newKey = params[paramIndex++];
					channel->setKey(newKey);
					appliedModes += "+k";
					appliedParams += " " + newKey;
				}
				else
					sendNumeric(client, "461", "MODE :Not enough parameters for +k");
			}
			else
			{
				if (!channel->getKey().empty())
				{
					channel->setKey("");
					appliedModes += "-k";
				}
			}
		}
		else if (mode == 'l')
		{
			if (adding)
			{
				if (paramIndex < params.size())
				{
					int limit = std::stoi(params[paramIndex++]);
					if (limit > 0)
					{
						channel->setMaxUsers(static_cast<std::size_t>(limit));
						appliedModes += "+l";
						appliedParams += " " + intToString(limit);
					}
				}
				else
					sendNumeric(client, "461", "MODE :Not enough parameters for +l");
			} else
			{
				if (channel->getMaxUsers() > 0)
				{
					channel->setMaxUsers(0);
					appliedModes += "-l";
				}
			}
		}
		else if (mode == 'o')
		{
			if (paramIndex < params.size())
			{
				std::string targetNick = params[paramIndex++];
				std::map<std::string, int>::iterator nickIt = _nicknames.find(targetNick);
				
				if (nickIt == _nicknames.end() || !channel->hasMember(nickIt->second))
					sendNumeric(client, "441", targetNick + " " + channel->getName() + " :They aren't on that channel");
				else
				{
					int targetFd = nickIt->second;
					if (adding)
					{
						if (!channel->isOperator(targetFd))
						{
							channel->addOperator(targetFd);
							appliedModes += "+o";
							appliedParams += " " + targetNick;
						}
					}
					else
					{
						if (channel->isOperator(targetFd))
						{
							channel->removeOperator(targetFd);
							appliedModes += "-o";
							appliedParams += " " + targetNick;
						}
					}
				}
			} else
				sendNumeric(client, "461", "MODE :Not enough parameters for o");
		}
		else
			sendNumeric(client, "472", std::string(1, mode) + " :is unknown mode char to me");
	}
	if (!appliedModes.empty())
	{
		std::string nick = client.getNickname();
		std::string user = client.getUsername();
		std::string modeBroadcast = ":" + nick + "!" + user + " MODE " + channel->getName() + " " + appliedModes + appliedParams + "\r\n";
		channel->broadcast(modeBroadcast);
	}
}