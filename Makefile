NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++20
CPPFLAGS := -Iinclude

SRCS := \
	src/main.cpp \
	src/Client.cpp \
	src/Message.cpp \
	src/Server.cpp \
	src/Channel.cpp \
	src/cmd/Join.cpp src/cmd/Who.cpp src/cmd/Privmsg.cpp src/cmd/Kick.cpp \
	src/cmd/Part.cpp src/cmd/Topic.cpp src/cmd/Mode.cpp src/cmd/Invite.cpp

OBJS := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
