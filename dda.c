/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dda.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 11:10:53 by abin-moh          #+#    #+#             */
/*   Updated: 2025/09/30 11:27:14 by abin-moh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

/*
    reset the value so it will not keep value from previous calc
*/
int init_ray(t_ray *ray)
{
    ray->hit = 0;
    ray->side = 0;
    ray->perp_wall_dist = 0.0;
    ray->line_height = 0;
    ray->draw_start = 0;
    ray->draw_end = 0;
    ray->tex = NULL;
    ray->tex_num = 0;
    ray->tex_pos = 0.0;
    ray->step = 0.0;
    return (0);
}

/*
    set the camera angle (-1 to 1)
    determine the ray direction in x and y
    get the player coordinate
    get the delta dist(distance to get to the next wall) 
*/
void	ray_setup(t_game *game, t_ray *ray, int x)
{
    ray->camera_x = 2.0 * x / (double)WIN_WIDTH - 1.0;
    ray->ray_dir_x = game->player.dir_x + game->player.plane_x * ray->camera_x;
    ray->ray_dir_y = game->player.dir_y + game->player.plane_y * ray->camera_x;
    ray->map_x = (int)game->player.pos_x;
    ray->map_y = (int)game->player.pos_y;
    if (ray->ray_dir_x == 0.0)
        ray->delta_dist_x = 1e30;
    else
        ray->delta_dist_x = fabs(1.0 / ray->ray_dir_x);
    if (ray->ray_dir_y == 0.0)
        ray->delta_dist_y = 1e30;
    else
        ray->delta_dist_y = fabs(1.0 / ray->ray_dir_y);
    ray_cont(game, ray);
}

/*
    determine the step x and y based on the ray direction
    (for adding it to the equation later)
    determine the side dist x and y to start the dda
*/
void    ray_cont(t_game *game, t_ray *ray)
{
    if (ray->ray_dir_x < 0)
    {
        ray->step_x = -1;
        ray->side_dist_x = (game->player.pos_x - ray->map_x) * ray->delta_dist_x;
    }
    else
    {
        ray->step_x = 1;
        ray->side_dist_x = (ray->map_x + 1.0 - game->player.pos_x) * ray->delta_dist_x;
    }
    if (ray->ray_dir_y < 0)
    {
        ray->step_y = -1;
        ray->side_dist_y = (game->player.pos_y - ray->map_y) * ray->delta_dist_y;
    }
    else
    {
        ray->step_y = 1;
        ray->side_dist_y = (ray->map_y + 1.0 - game->player.pos_y) * ray->delta_dist_y;
    }
}

/*
    perform the DDA loop (Digital Differential Analyzer)
    step through the map grid until the ray hits a wall
    compare side_dist_x and side_dist_y to choose step direction
    update map_x or map_y depending on which side was crossed
    stop when a wall ('1') is hit or ray goes out of bounds
    set ray->side to know if it hit vertical (0) or horizontal (1)
*/
void	dda_loop(t_game *game, t_ray *ray)
{
    while (ray->hit == 0)
    {
        if (ray->side_dist_x < ray->side_dist_y)
        {
            ray->side_dist_x += ray->delta_dist_x;
            ray->map_x += ray->step_x;
            ray->side = 0;
        }
        else
        {
            ray->side_dist_y += ray->delta_dist_y;
            ray->map_y += ray->step_y;
            ray->side = 1;
        }
        if (ray->map_y < 0 || ray->map_y >= game->map_height ||
            ray->map_x < 0 || ray->map_x >= game->map_width)
        {
            ray->hit = 1;
            break ;
        }
        if (game->map[ray->map_y][ray->map_x] == '1')
        ray->hit = 1;
    }
}

/*
    calculate perpendicular distance from player to the wall
    avoid division by zero by using a very small value (1e-6)
    adjust distance depending on which side (x or y) was hit
    compute line height based on distance to scale wall slice
    determine draw_start and draw_end to know where to render
    clamp draw range so it stays inside the screen height
*/

void	distance_to_wall(t_game *game, t_ray *ray)
{
    if (ray->side == 0)
    {
        if (ray->ray_dir_x == 0.0)
            ray->perp_wall_dist = 1e-6;
        else
            ray->perp_wall_dist = (ray->map_x - game->player.pos_x
                + (1 - ray->step_x) / 2.0) / ray->ray_dir_x;
    }
    else
    {
        if (ray->ray_dir_y == 0.0)
            ray->perp_wall_dist = 1e-6;
        else
            ray->perp_wall_dist = (ray->map_y - game->player.pos_y
                + (1 - ray->step_y) / 2.0) / ray->ray_dir_y;
    }
    if (ray->perp_wall_dist <= 0.0)
        ray->perp_wall_dist = 1e-6;
    ray->line_height = (int)(WIN_HEIGHT / ray->perp_wall_dist);
    ray->draw_start = -ray->line_height / 2 + WIN_HEIGHT / 2;
    if (ray->draw_start < 0)
        ray->draw_start = 0;
    ray->draw_end = ray->line_height / 2 + WIN_HEIGHT / 2;
    if (ray->draw_end >= WIN_HEIGHT)
        ray->draw_end = WIN_HEIGHT - 1;
}
