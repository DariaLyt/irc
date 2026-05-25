NAME = ircserv

C++ = c++
C++_FLAGS = -Wall -Wextra -Werror -std=c++20
SRCS = src/main.cpp

OBJS = $(SRCS:.cpp=.o)


all: $(NAME)

$(NAME):$(OBJS)
	$(C++) $(C++_FLAGS) $(OBJS) -o $(NAME) 

%.o: %.cpp
	$(C++) $(C++_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re