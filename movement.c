/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   movement.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 11:05:52 by abin-moh          #+#    #+#             */
/*   Updated: 2025/09/30 11:06:50 by abin-moh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

void strafe_right(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)]
        [(int)(game->player.pos_x + game->player.plane_x * MOVE_SPEED)] == '0')
        game->player.pos_x += game->player.plane_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y + game->player.plane_y * MOVE_SPEED)]
        [(int)(game->player.pos_x)] == '0')
        game->player.pos_y += game->player.plane_y * MOVE_SPEED;
}


void rotate_left(t_game *game)
{
    double old_dir_x = game->player.dir_x;
    game->player.dir_x = game->player.dir_x
        * cos(ROT_SPEED) - game->player.dir_y * sin(ROT_SPEED);
    game->player.dir_y = old_dir_x * sin(ROT_SPEED)
        + game->player.dir_y * cos(ROT_SPEED);
    double old_plane_x = game->player.plane_x;
    game->player.plane_x = game->player.plane_x * cos(ROT_SPEED)
        - game->player.plane_y * sin(ROT_SPEED);
    game->player.plane_y = old_plane_x * sin(ROT_SPEED)
        + game->player.plane_y * cos(ROT_SPEED);
}

void rotate_right(t_game *game)
{
    double old_dir_x = game->player.dir_x;
    game->player.dir_x = game->player.dir_x * cos(-ROT_SPEED)
        - game->player.dir_y * sin(-ROT_SPEED);
    game->player.dir_y = old_dir_x * sin(-ROT_SPEED)
        + game->player.dir_y * cos(-ROT_SPEED);

    double old_plane_x = game->player.plane_x;
    game->player.plane_x = game->player.plane_x * cos(-ROT_SPEED)
        - game->player.plane_y * sin(-ROT_SPEED);
    game->player.plane_y = old_plane_x * sin(-ROT_SPEED)
        + game->player.plane_y * cos(-ROT_SPEED);
}

int key_press(int keycode, t_game *game)
{
    if (keycode >= 0 && keycode < 256)
        game->keys[keycode] = 1;
    if (keycode == KEY_LEFT)
        game->key_left = 1;
    if (keycode == KEY_RIGHT)
        game->key_right = 1;
    if (keycode == KEY_ESC)
        exit(1);                 /* kene check exit nak kena cleanup apa2 tak */
    return (0);
}

int key_release(int keycode, t_game *game)
{
    if (keycode >= 0 && keycode < 256)
        game->keys[keycode] = 0;
    if (keycode == KEY_LEFT)
        game->key_left = 0;
    if (keycode == KEY_RIGHT)
        game->key_right = 0;
    return (0);
}