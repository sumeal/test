/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dda2.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 11:20:27 by abin-moh          #+#    #+#             */
/*   Updated: 2025/09/30 11:36:00 by abin-moh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

/*
    choose which wall texture to use based on ray direction
    if hit on x-side, check whether ray is facing left or right
    if hit on y-side, check whether ray is facing up or down
    assign the correct texture index (0–3)
    store pointer to the chosen texture for rendering
*/

void	pick_texture(t_game *game, t_ray *ray)
{
    if (ray->side == 0 && ray->ray_dir_x > 0)
        ray->tex_num = 3;
    else if (ray->side == 0 && ray->ray_dir_x < 0)
        ray->tex_num = 2;
    else if (ray->side == 1 && ray->ray_dir_y > 0)
        ray->tex_num = 1;
    else
        ray->tex_num = 0;
    ray->tex = &game->textures[ray->tex_num].img;
}

/*
    calculate exact point on the wall where the ray hit (wall_x)
    convert that point into a horizontal texture coordinate (tex_x)
    adjust tex_x if the ray is facing the opposite side
    calculate how much texture to move per screen pixel (step)
    find the starting texture position at the top of the wall slice (tex_pos)
*/
void texture_and_coordinate(t_game *game, t_ray *ray)
{
    if (!ray->tex || ray->tex->width <= 0 || ray->tex->height <= 0 || !ray->tex->addr)
        return ;
    if (ray->side == 0)
        ray->wall_x = game->player.pos_y + ray->perp_wall_dist * ray->ray_dir_y;
    else
        ray->wall_x = game->player.pos_x + ray->perp_wall_dist * ray->ray_dir_x;
    ray->wall_x -= floor(ray->wall_x);
    ray->tex_x = (int)(ray->wall_x * (double)ray->tex->width);
    if ((ray->side == 0 && ray->ray_dir_x > 0) || (ray->side == 1 && ray->ray_dir_y < 0))
        ray->tex_x = ray->tex->width - ray->tex_x - 1;
    if (ray->tex_x < 0)
		ray->tex_x = 0;
    if (ray->tex_x >= ray->tex->width) ray->tex_x = ray->tex->width - 1;
    ray->step = 1.0 * ray->tex->height / (double)ray->line_height;
    ray->tex_pos = (ray->draw_start - WIN_HEIGHT / 2.0 + ray->line_height / 2.0) * ray->step;
}

/*
    draw the textured wall slice column by column
    loop from the start (draw_start) to the end (draw_end) of the wall height
    for each pixel:
        - calculate the corresponding y coordinate in the texture (tex_y)
        - read the color from the texture at (tex_x, tex_y)
        - place that color into the screen image buffer at (x, y)
    increment tex_pos each time to move down the texture correctly
*/
void draw_vertical_line(t_game *game, t_ray *ray, int x)
{
    char            *dst;
    unsigned int    color;
    char            *tex_dst;
    int             y;
    int             tex_y;

    if (!ray->tex || !ray->tex->addr)
        return ;
    y = ray->draw_start - 1;
    while (++y < ray->draw_end)
    {
        tex_y = (int)ray->tex_pos;
        if (tex_y < 0)
            tex_y = 0;
        if (tex_y >= ray->tex->height)
            tex_y = ray->tex->height - 1;
        ray->tex_pos += ray->step;
        tex_dst = ray->tex->addr + (tex_y * ray->tex->line_length
                    + ray->tex_x * (ray->tex->bits_per_pixel / 8));
        color = *(unsigned int *)tex_dst;
        dst = game->img.addr + (y * game->img.line_length
                        + x * (game->img.bits_per_pixel / 8));
        *(unsigned int *)dst = color;
    }
}
