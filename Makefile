# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/25 09:48:14 by abin-moh          #+#    #+#              #
#    Updated: 2025/10/03 11:04:29 by abin-moh         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# --- Project Name ---
NAME = cub3D

# --- Compiler ---
CC = cc
CFLAGS = -Wall -Wextra -Werror -Imlx_linux -Ilibft

# --- Sources & Objects ---
SRCS = main.c \
	   render.c \
	   dda.c \
	   dda2.c \
	   movement.c \
	   movement2.c
	   
OBJS = $(SRCS:.c=.o)

# --- MiniLibX ---
MLX_DIR = mlx_linux
MLX_LIB = $(MLX_DIR)/libmlx.a
MLX_FLAGS = -L$(MLX_DIR) -lmlx -lXext -lX11 -lm -lz

# --- Libft ---
LIBFT_DIR = libft
LIBFT_LIB = $(LIBFT_DIR)/libft.a

# --- Rules ---
all: $(NAME)

$(NAME): $(OBJS) $(MLX_LIB) $(LIBFT_LIB)
	$(CC) $(CFLAGS) $(OBJS) $(MLX_FLAGS) -L$(LIBFT_DIR) -lft -o $(NAME)

$(MLX_LIB):
	@$(MAKE) -C $(MLX_DIR)

$(LIBFT_LIB):
	@$(MAKE) -C $(LIBFT_DIR)

clean:
	rm -f $(OBJS)
	@$(MAKE) -C $(MLX_DIR) clean
	@$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	rm -f $(NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

.PHONY: all clean fclean re
