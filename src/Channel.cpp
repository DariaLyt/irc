#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name) {}

Channel::~Channel() {}

const std::string &Channel::getName() const
{
	return (_name);
}

const std::map<int, Client *> &Channel::getMembers() const
{
	return (_members);
}

void Channel::addMember(Client *client)
{
	_members[client->getFd()] = client;
}

void Channel::removeMember(int fd)
{
	_members.erase(fd);
}

bool Channel::hasMember(int fd) const
{
	return (_members.find(fd) != _members.end());
}

bool Channel::isEmpty() const
{
	return (_members.empty());
}

void Channel::broadcast(const std::string &msg, int excludeFd)
{
	for (std::map<int, Client *>::iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (it->first != excludeFd)
			it->second->queueOutput(msg);
	}
}