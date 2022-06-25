NAME_FULL	= vfs_test
NAME_FUNC	= func_test
FLAGS		= -Wall -Wextra -Werror -g -fsanitize=address -std=c++11
INCLUDES	= -I../
SRCS		= ../VFS.cpp
FULLTEST	= test.cpp
FUNCTEST	= function_test.cpp
CC			= c++

all: $(NAME_FULL)

$(NAME_FULL): $(SRCS) $(FULLTEST)
	$(CC) $(FLAGS) $(INCLUDES) $(SRCS) $(FULLTEST) -o $(NAME_FULL)

func: $(NAME_FUNC)

$(NAME_FUNC): $(SRCS) $(FUNCTEST)
	$(CC) $(FLAGS) $(INCLUDES) $(SRCS) $(FUNCTEST) -o $(NAME_FUNC)

clean:
	rm VFS_File VFS_Table ../VFS_Table ../VFS_File

fclean:
	rm -f $(NAME_FULL) $(NAME_FUNC)

re: fclean all

.PHONY: all fclean re func clean