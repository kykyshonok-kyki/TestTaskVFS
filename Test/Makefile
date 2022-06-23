NAME	= vfs_test
FLAGS	= -Wall -Wextra -Werror -g -fsanitize=address
INCLUDES= -I../
SRCS	= ../VFS.cpp test.cpp
CC		= c++

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(FLAGS) $(INCLUDES) $(SRCS) -o $(NAME)

clean:
	rm VFS_File VFS_Table ../VFS_Table ../VFS_File

fclean:
	rm -f $(NAME)

re: fclean all

.PHONY: all fclean re