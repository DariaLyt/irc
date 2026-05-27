#pragma once

#include "Client.hpp"
#include <map>
#include <string>

class Channel
{
	private:
		std::string _name;
		std::map<int, Client*> _members;
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
};