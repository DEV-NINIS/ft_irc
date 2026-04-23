CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc

ifeq ($(OS),Windows_NT)
LDLIBS += -lws2_32
endif

SRCS = src/main.cpp \
	src/Server.cpp \
	src/Client.cpp \
	src/Chanel.cpp \
	src/Command.cpp \
	src/Message.cpp \
	src/ParsingUtils.cpp \
	src/utils.cpp
OBJS = $(SRCS:src/%.cpp=obj/%.o)
NAME = ircserv

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
