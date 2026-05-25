#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

class Client
{
	public:
		explicit Client(int fd);

		int getFd() const;

		bool hasPass() const;
		bool isRegistered() const;
		const std::string &getNickname() const;
		const std::string &getUsername() const;

		void setPassOk(bool value);
		void setRegistered(bool value);
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);

		void appendInput(const char *data, std::size_t length);
		bool hasCompleteLine() const;
		std::string popLine();

		void queueOutput(const std::string &message);
		bool hasOutput() const;
		const std::string &getOutput() const;
		void consumeOutput(std::size_t length);

	private:
		int _fd;
		bool _passOk;
		bool _registered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _input;
		std::string _output;

		Client();
};

#endif
