#include "Server.hpp"

void Server::handleNick(Client &client, const Message &message)
{
	const std::vector<std::string> &params = message.getParams();

	if (params.empty())
	{
		sendNumeric(client, "431", ":No nickname given");
		return;
	}
	if (!isValidNickname(params[0]))
	{
		sendNumeric(client, "432", params[0] + " :Erroneous nickname");
		return;
	}
	if (nicknameInUse(params[0], client.getFd()))
	{
		sendNumeric(client, "433", params[0] + " :Nickname is already in use");
		return;
	}
	std::string oldNick = client.getNickname();
	if (!oldNick.empty())
		_nicknames.erase(oldNick);
	client.setNickname(params[0]);
	_nicknames[params[0]] = client.getFd();
	if (client.isRegistered() && !oldNick.empty() && oldNick != params[0])
	{
		std::string nickMsg = ":" + oldNick + "!" + client.getUsername()
			+ " NICK :" + params[0] + "\r\n";
		sendReply(client, nickMsg);
		for (std::map<std::string, Channel *>::iterator it = _channels.begin();
			it != _channels.end(); ++it)
		{
			if (it->second->hasMember(client.getFd()))
				it->second->broadcast(nickMsg, client.getFd());
		}
	}
	maybeRegister(client);
}
