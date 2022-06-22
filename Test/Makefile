NAME	= ivfs_test
FLAGS	= -Wall -Wextra -Werror
INCLUDES= -I../
SRCS	= ../VFS.cpp test.cpp
CC		= c++

all: $(NAME)

$(NAME):
	$(CC) $(FLAGS) $(INCLUDES) $(SRCS) -o $(NAME)

fclean:
	rm -f $(NAME)

re: fclean all

.PHONY: all fclean re