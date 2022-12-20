NAME		=	ircserv
INC_DIR		=	include
SRC_DIR		=	sources
OBJ_DIR		=	objects

CXX			=	c++
CPPFLAGS	=	-std=c++98 -g3 -Wall -Wextra -Werror
DELETE		=	rm -rf

SRCS		=	main.cpp \
		        Server.cpp
OBJS		=	$(addprefix $(OBJ_DIR)/, $(SRCS:%cpp=%o))

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
			@mkdir -p $(OBJ_DIR)
			@$(CXX) $(CPPFLAGS) -I$(INC_DIR) -c $< -o $@

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CXX) $(CPPFLAGS) $(OBJS) -o $@

irssi: rm_irssi
	docker run -it --name my-running-irssi -e TERM -u $(id -u):$(id -g) \
	--log-driver=none \
    -v ${HOME}/.irssi:/home/user/.irssi:ro \
    irssi

rm_irssi:
	@docker rm -f my-running-irssi 2> /dev/null || exit 0

clean:
			@$(DELETE) $(OBJ_DIR)

fclean:		clean
			@$(DELETE) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
