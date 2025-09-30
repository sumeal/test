/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 11:04:23 by abin-moh          #+#    #+#             */
/*   Updated: 2025/09/30 11:10:05 by abin-moh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int render_frame(t_game *game)
{
    render_movement(game);
    render_ceiling_floor(game);
    render_walls(game);
    mlx_put_image_to_window(game->mlx, game->win, game->img.img, 0, 0);
    return (0);
}

int render_movement(t_game *game)
{
    if (game->keys[KEY_W])
        move_forward(game);
    if (game->keys[KEY_S])
        move_backward(game);
    if (game->keys[KEY_A])
        strafe_left(game);
    if (game->keys[KEY_D])
        strafe_right(game);
    if (game->key_left)
        rotate_right(game);
    if (game->key_right)
        rotate_left(game);
    return (0);
}

int render_ceiling_floor(t_game *game)
{
    int             y;
    int             x;
    char            *dst;
    unsigned int    color;

    y = -1;
    while (++y < WIN_HEIGHT)
    {
        if (y < WIN_HEIGHT / 2)
            color = game->ceiling_color;
        else
            color = game->floor_color;
        x = -1;
        while (++x < WIN_WIDTH)
        {
            dst = game->img.addr + (y * game->img.line_length + x * (game->img.bits_per_pixel / 8));
            *(unsigned int *)dst = color;
        }
    }
	return (0);
}

void    render_walls(t_game *game)
{
	t_ray   ray;
    int     x;

    x = -1;
    while (++x < WIN_WIDTH)
    {
        init_ray(&ray);
        ray_setup(game, &ray, x);
        dda_loop(game, &ray);
        distance_to_wall(game, &ray);
        pick_texture(game, &ray);
        texture_and_coordinate(game, &ray);
        draw_vertical_line(game, &ray, x);
    }
}

