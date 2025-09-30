#include "cub3d.h"
#include <mlx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for memset, strlen
#include <math.h>

#define WIN_WIDTH 1980
#define WIN_HEIGHT 1080

// --- Utility (if you don't have libft yet) ---
static void	ft_bzero(void *s, size_t n)
{
	memset(s, 0, n);
}

// --- Close window ---
int	close_window(t_game *game)
{
	if (game->win)
		mlx_destroy_window(game->mlx, game->win);
	exit(0);
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
    set the camera angle (-1 to 1)
    determine the ray direction in x and y
    get the player coordinate
    get the delta dist(distance to get to the next wall) 
*/
void ray_setup(t_game *game, t_ray *ray, int x)
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
    perform the DDA loop (Digital Differential Analyzer)
    step through the map grid until the ray hits a wall
    compare side_dist_x and side_dist_y to choose step direction
    update map_x or map_y depending on which side was crossed
    stop when a wall ('1') is hit or ray goes out of bounds
    set ray->side to know if it hit vertical (0) or horizontal (1)
*/
void dda_loop(t_game *game, t_ray *ray)
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

void distance_to_wall(t_game *game, t_ray *ray)
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

/*
    choose which wall texture to use based on ray direction
    if hit on x-side, check whether ray is facing left or right
    if hit on y-side, check whether ray is facing up or down
    assign the correct texture index (0–3)
    store pointer to the chosen texture for rendering
*/

void pick_texture(t_game *game, t_ray *ray)
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


// --- Setup player defaults ---
void	init_player(t_player *p) /*mad*/
{
	p->pos_x = 3.5;   // player position (x)
	p->pos_y = 3.5;   // player position (y)
	p->dir_x = 1.0;  // facing left (east)
	p->dir_y = 0.0;
	p->plane_x = 0.0;
	p->plane_y = 0.66; // FOV
}

// --- Setup mlx image buffer ---
void	init_image(t_game *game)
{
	game->img.img = mlx_new_image(game->mlx, WIN_WIDTH, WIN_HEIGHT);
	game->img.addr = mlx_get_data_addr(game->img.img,
			&game->img.bits_per_pixel,
			&game->img.line_length,
			&game->img.endian);
	game->img.width = WIN_WIDTH;
	game->img.height = WIN_HEIGHT;
	memset(game->keys, 0, sizeof(game->keys));
	game->key_left = 0;
	game->key_right = 0;

}

// --- Load one texture from XPM ---
void	load_texture(t_game *game, t_texture *tex, char *path)
{
	tex->img.img = mlx_xpm_file_to_image(game->mlx, path,
			&tex->img.width, &tex->img.height);
	if (!tex->img.img)
	{
		fprintf(stderr, "Error: failed to load texture %s\n", path);
		exit(1);
	}
	tex->img.addr = mlx_get_data_addr(tex->img.img,
			&tex->img.bits_per_pixel,
			&tex->img.line_length,
			&tex->img.endian);
}

// --- Load all 4 textures ---
void	init_textures(t_game *game)
{
	load_texture(game, &game->textures[0], "textures/north.xpm");
	load_texture(game, &game->textures[1], "textures/south.xpm");
	load_texture(game, &game->textures[2], "textures/west.xpm");
	load_texture(game, &game->textures[3], "textures/east.xpm");
}

// --- Default floor & ceiling colors ---
void	init_colors(t_game *game)
{
	game->floor_color = 0x333333;   // dark gray
	game->ceiling_color = 0x87CEEB; // sky blue
}

// --- Default test map ---
char *default_map[] = {
	"1111111111",
	"1000000101",
	"1000110001",
	"1100000101",
	"1000100001",
	"1000000001",
	"1111111111",
	NULL
};

// --- Attach default map to game ---
void	init_map(t_game *game)
{
	game->map = default_map;
	game->map_width = strlen(default_map[0]);
	int h = 0;
	while (default_map[h])
		h++;
	game->map_height = h;
}

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

int render_frame(t_game *game)
{
    render_movement(game);
    render_ceiling_floor(game);
    render_walls(game);
    mlx_put_image_to_window(game->mlx, game->win, game->img.img, 0, 0);
    return (0);
}


// --- Main ---
int	main(void)
{
	t_game	game;

	ft_bzero(&game, sizeof(t_game));

	// Init MLX 
    /*mad*/
	game.mlx = mlx_init();
	if (!game.mlx)
		return (fprintf(stderr, "Error: mlx_init failed\n"), 1);

	// Create window
	game.win = mlx_new_window(game.mlx, WIN_WIDTH, WIN_HEIGHT, "cub3D");
	if (!game.win)
		return (fprintf(stderr, "Error: mlx_new_window failed\n"), 1);

	// Init
	init_player(&game.player);
	init_image(&game);
	init_textures(&game);
	init_map(&game);
	init_colors(&game);

	// Hooks
	mlx_hook(game.win, 17, 0, close_window, &game);
	mlx_hook(game.win, 2, 1L<<0, key_press, &game);
	mlx_hook(game.win, 3, 1L<<1, key_release, &game);
	mlx_loop_hook(game.mlx, render_frame, &game);

	// Game loop
	mlx_loop(game.mlx);

	return (0);
}
