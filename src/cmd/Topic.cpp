#include "Server.hpp"

void Server::handleTopic(Client &client, const Message &message)
{
    const std::vector<std::string> &params = message.getParams();

    if (!client.isRegistered()) {
        sendNumeric(client, "451", ":You have not registered");
        return;
    }
    if (params.empty()) {
        sendNumeric(client, "461", "TOPIC :Not enough parameters");
        return;
    }

    std::string channelName = params[0];
    std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
    if (it == _channels.end()) {
        sendNumeric(client, "403", channelName + " :No such channel");
        return;
    }

    Channel *channel = it->second;
    if (!channel->hasMember(client.getFd())) {
        sendNumeric(client, "442", channelName + " :You're not on that channel");
        return;
    }

    if (params.size() == 1) {
		if (channel->getTopic().empty()) {
			sendNumeric(client, "331", channelName + " :No topic is set");
		} else {
			sendNumeric(client, "332", channelName + " :" + channel->getTopic());
		}
		return;
	}
	if (channel->isTopicRestricted() && !channel->isOperator(client.getFd())){
		sendNumeric(client, "482", channelName + " :You're not channel operator");
		return;
	}

	std::string newTopic = params[1];
	channel->setTopic(newTopic);
	std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";
	channel->broadcast(topicMsg);
}