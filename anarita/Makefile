NAME = irc
CXX  = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = server.cpp
OBJS = $(SRCS:.cpp=.o)
HEADERS = $(wildcard *.hpp)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS): $(HEADERS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re