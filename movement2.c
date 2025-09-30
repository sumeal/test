/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   movement2.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 11:07:00 by abin-moh          #+#    #+#             */
/*   Updated: 2025/09/30 11:07:23 by abin-moh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

void move_forward(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)]
        [(int)(game->player.pos_x + game->player.dir_x * MOVE_SPEED)] == '0')
        game->player.pos_x += game->player.dir_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y + game->player.dir_y * MOVE_SPEED)]
        [(int)(game->player.pos_x)] == '0')
        game->player.pos_y += game->player.dir_y * MOVE_SPEED;
}

void move_backward(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)]
        [(int)(game->player.pos_x - game->player.dir_x * MOVE_SPEED)] == '0')
        game->player.pos_x -= game->player.dir_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y - game->player.dir_y * MOVE_SPEED)]
        [(int)(game->player.pos_x)] == '0')
        game->player.pos_y -= game->player.dir_y * MOVE_SPEED;
}

void strafe_left(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)]
        [(int)(game->player.pos_x - game->player.plane_x * MOVE_SPEED)] == '0')
        game->player.pos_x -= game->player.plane_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y - game->player.plane_y * MOVE_SPEED)]
        [(int)(game->player.pos_x)] == '0')
        game->player.pos_y -= game->player.plane_y * MOVE_SPEED;
}