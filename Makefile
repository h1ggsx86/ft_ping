CC		= cc
CFLAGS	= -Wall -Wextra -Werror -g3

NAME	= ft_ping

OBJ_DIR		= obj
INC_DIR		= includes
SRC_DIR		= srcs

SRC		= $(SRC_DIR)/parsing.c $(SRC_DIR)/utils.c $(SRC_DIR)/main.c
SRC_OBJ	= $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(SRC_OBJ)
		$(CC) $(CFLAGS) -I $(INC_DIR) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
		@mkdir -p $(dir $@)
		$(CC) $(CFLAGS) -I $(INC_DIR) -O3 -c $< -o $@

$(OBJ_DIR):
		mkdir -p $@

clean:
		@echo "[CLEAN]: "
		@if [ ! -d "$(OBJ_DIR)" ]; \
		then \
			echo "Repo '.o' already clean"; echo ; \
		else \
			rm -rfv $(OBJ_DIR); \
			echo ;\
		fi
		@echo

fclean: clean
		@echo "[FCLEAN]: "
		@if [ -f "$(NAME)" ]; \
		then \
			rm -fv $(NAME); \
		else \
			echo "Executable already removed"; echo; \
		fi

re: fclean all

.PHONY: all clean fclean re