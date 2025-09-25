# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/25 09:48:14 by abin-moh          #+#    #+#              #
#    Updated: 2025/09/25 09:49:39 by abin-moh         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# --- Project Name ---
NAME = cub3D

# --- Compiler ---
CC = cc
CFLAGS = -Wall -Wextra -Werror -Imlx_linux

# --- Sources & Objects ---
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# --- MiniLibX ---
MLX_DIR = mlx_linux
MLX_LIB = $(MLX_DIR)/libmlx.a
MLX_FLAGS = -L$(MLX_DIR) -lmlx -lXext -lX11 -lm -lz

# --- Rules ---
all: $(NAME)

$(NAME): $(OBJS) $(MLX_LIB)
	$(CC) $(CFLAGS) $(OBJS) $(MLX_FLAGS) -o $(NAME)

$(MLX_LIB):
	@$(MAKE) -C $(MLX_DIR)

clean:
	rm -f $(OBJS)
	@$(MAKE) -C $(MLX_DIR) clean

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
