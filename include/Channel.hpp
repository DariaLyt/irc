#pragma once

#include "Client.hpp"
#include <map>
#include <string>
#include <vector>

class Channel
{
	private:
		std::string _name;
		std::map<int, Client*> _members;
		std::string _topic;
		std::vector<int> _operators; // operator fds
		bool _inviteOnly;
		bool _topicResticted;
		std::string _key;
		std::size_t _maxUsers; // if 0 - no limit
		std::vector<int> _invitedFds;
		Channel();

	public:
		explicit Channel(const std::string &name);
		~Channel();
		
		const std::string &getName() const;
		const std::map<int, Client *> &getMembers() const;

		void addMember(Client *client);
		void removeMember(int fd);
		bool hasMember(int fd) const;
		bool isEmpty() const;
		void broadcast(const std::string &message, int excludeFd= -1);
		
		const std::string &getTopic() const;
		void setTopic(const std::string &topic);
		void addOperator(int fd);
		void removeOperator(int fd);
		bool isOperator(int fd) const;

		bool isInviteOnly() const;
		void setInviteOnly(bool value);

		bool isTopicRestricted() const;
		void setTopicRestricted(bool value);

		const std::string &getKey() const;
		void setKey(const std::string &key);

		std::size_t getMaxUsers() const;
		void setMaxUsers(std::size_t limit);
		void addInvite(int fd);
		void removeInvite(int fd);
		bool isInvited(int fd) const;
};