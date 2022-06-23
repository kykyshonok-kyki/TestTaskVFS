NAME	= ivfs_test
#FLAGS	= -Wall -Wextra -Werror
INCLUDES= -I../
SRCS	= ../VFS.cpp test.cpp
CC		= c++

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(FLAGS) $(INCLUDES) $(SRCS) -o $(NAME)

clean:
	rm VFS_File VFS_Table

fclean:
	rm -f $(NAME)

re: fclean all

.PHONY: all fclean re