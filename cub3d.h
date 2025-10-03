/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cub3d.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abin-moh <abin-moh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 09:48:59 by abin-moh          #+#    #+#             */
/*   Updated: 2025/10/03 11:06:45 by abin-moh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CUB3D_H
#define CUB3D_H

#define KEY_ESC     65307
#define KEY_W       119
#define KEY_A       97
#define KEY_S       115
#define KEY_D       100
#define KEY_LEFT    65361
#define KEY_RIGHT   65363
#define MOVE_SPEED 0.05
#define ROT_SPEED 0.05

#define WIN_WIDTH 1980
#define WIN_HEIGHT 1080

#include "./libft/libft.h"
#include <mlx.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


typedef struct s_img
{
	void	*img;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
	int		width;
	int		height;
}	t_img;

typedef struct s_texture
{
	t_img			img;
	unsigned int	**texture;
}	t_texture;

typedef struct s_player
{
	double	pos_x;
	double	pos_y;
	double	dir_x;
	double	dir_y;
	double	plane_x;
	double	plane_y;
}	t_player;

typedef struct s_ray
{
	double	camera_x;
	double	ray_dir_x;
	double	ray_dir_y;
	int		map_x;
	int		map_y;
	double	side_dist_x;
	double	side_dist_y;
	double	delta_dist_x;
	double	delta_dist_y;
	double	perp_wall_dist;
	int		step_x;
	int		step_y;
	int		hit;
	int		side;
	int		line_height;
	int		draw_start;
	int		draw_end;
	int		tex_num;
	double	wall_x;
	int		tex_x;
	int		tex_y;
	double	step;
	double	tex_pos;
	t_img	*tex;
}	t_ray;

typedef struct s_game
{
	void		*mlx;
	void		*win;
	t_img		img;
	char		**map;
	int			map_width;
	int			map_height;
	t_player	player;
	t_texture	textures[4];
	char		*north_texture;
	char		*south_texture;
	char		*west_texture;
	char		*east_texture;
	int			floor_color;
	int			ceiling_color;
	int			keys[256];
	int			key_left;
	int			key_right;
}	t_game;

/*render.c*/
int		render_frame(t_game *game);
int		render_movement(t_game *game);
int		render_ceiling_floor(t_game *game);
void	render_walls(t_game *game);

/*dda.c*/
int		init_ray(t_ray *ray);
void	ray_setup(t_game *game, t_ray *ray, int x);
void	ray_cont(t_game *game, t_ray *ray);
void	dda_loop(t_game *game, t_ray *ray);
void	distance_to_wall(t_game *game, t_ray *ray);

/*dda2.c*/
void	pick_texture(t_game *game, t_ray *ray);
void	texture_and_coordinate(t_game *game, t_ray *ray);
void	draw_vertical_line(t_game *game, t_ray *ray, int x);

/*movement.c*/
void	strafe_right(t_game *game);
void	rotate_left(t_game *game);
void	rotate_right(t_game *game);
int		key_press(int keycode, t_game *game);
int		key_release(int keycode, t_game *game);

/*movement2.c*/
void	move_forward(t_game *game);
void	move_backward(t_game *game);
void	strafe_left(t_game *game);

#endif