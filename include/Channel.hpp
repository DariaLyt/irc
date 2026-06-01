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
};