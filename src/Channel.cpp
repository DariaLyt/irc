#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name), _inviteOnly(false), _topicResticted(true), _key(""), _maxUsers(0)  {}

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

const std::string &Channel::getTopic() const
{
	return (_topic);
}

void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

void Channel::addOperator(int fd)
{
	if (!isOperator(fd))
		_operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
	for (std::vector<int>::iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (*it == fd)
		{
			_operators.erase(it);
			break;
		}
	}
}

bool Channel::isOperator(int fd) const
{
	for (std::size_t i = 0; i < _operators.size(); ++i)
	{
		if (_operators[i] == fd)
			return (true);
	}
	return (false);
}

bool Channel::isInviteOnly() const
{
	return (_inviteOnly);
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

bool Channel::isTopicRestricted() const
{
	return (_topicResticted);
}

void Channel::setTopicRestricted(bool value)
{
	_topicResticted = value;
}

const std::string &Channel::getKey() const
{
	return (_key);
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}

std::size_t Channel::getMaxUsers() const 
{
	return (_maxUsers);
}

void Channel::setMaxUsers(std::size_t limit)
{
	_maxUsers = limit;
}

void Channel::addInvite(int fd)
{
	if (!isInvited(fd))
		_invitedFds.push_back(fd);
}

void Channel::removeInvite(int fd)
{
	for (std::vector<int>::iterator it = _invitedFds.begin(); it != _invitedFds.end(); it++)
	{
		if (*it == fd)
		{
			_invitedFds.erase(it);
			break;
		}
	}
}

bool Channel::isInvited(int fd) const
{
	for (std::size_t i = 0; i <_invitedFds.size(); ++i)
	{
		if (_invitedFds[i] == fd)
			return (true);
	}
	return (false);
}