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
	for (int y = 0; y < WIN_HEIGHT;y++)
	{
		unsigned int color = (y < WIN_HEIGHT / 2) ? game->ceiling_color : game->floor_color;
		for (int x = 0; x < WIN_WIDTH; x++)
		{
			char *dst = game->img.addr + (y * game->img.line_length + x * (game->img.bits_per_pixel / 8));
			*(unsigned int *)dst = color;
		}
	}
	return (0);
}

/* corrected ray_setup: store into ray-> fields (not locals) */
void ray_setup(t_game *game, t_ray *ray, int x)
{
    ray->camera_x = 2.0 * x / (double)WIN_WIDTH - 1.0;
    ray->ray_dir_x = game->player.dir_x + game->player.plane_x * ray->camera_x;
    ray->ray_dir_y = game->player.dir_y + game->player.plane_y * ray->camera_x;

    ray->map_x = (int)game->player.pos_x;
    ray->map_y = (int)game->player.pos_y;

    ray->delta_dist_x = (ray->ray_dir_x == 0.0) ? 1e30 : fabs(1.0 / ray->ray_dir_x);
    ray->delta_dist_y = (ray->ray_dir_y == 0.0) ? 1e30 : fabs(1.0 / ray->ray_dir_y);

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

    /* initialize other ray fields defensively */
    ray->hit = 0;
    ray->side = 0;
    ray->perp_wall_dist = 0.0;
    ray->line_height = 0;
    ray->draw_start = 0;
    ray->draw_end = 0;
}

/* corrected dda_loop: update ray->side, bounds-check map access */
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
        /* bounds check before accessing map */
        if (ray->map_y < 0 || ray->map_y >= game->map_height ||
            ray->map_x < 0 || ray->map_x >= game->map_width)
        {
            /* treat out-of-bounds as a hit to stop the ray (or you can break and skip drawing) */
            ray->hit = 1;
            break;
        }
        if (game->map[ray->map_y][ray->map_x] == '1')
            ray->hit = 1;
    }
}

/* corrected distance_to_wall: store computed values into ray-> fields */
void distance_to_wall(t_game *game, t_ray *ray)
{
    /* compute perp_wall_dist and guard against division by zero */
    if (ray->side == 0)
    {
        if (ray->ray_dir_x == 0.0)
            ray->perp_wall_dist = 1e-6;
        else
            ray->perp_wall_dist = (ray->map_x - game->player.pos_x + (1 - ray->step_x) / 2.0) / ray->ray_dir_x;
    }
    else
    {
        if (ray->ray_dir_y == 0.0)
            ray->perp_wall_dist = 1e-6;
        else
            ray->perp_wall_dist = (ray->map_y - game->player.pos_y + (1 - ray->step_y) / 2.0) / ray->ray_dir_y;
    }
	/* safety check */
    if (ray->perp_wall_dist <= 0.0)
        ray->perp_wall_dist = 1e-6;

	/* the wall is */
    ray->line_height = (int)(WIN_HEIGHT / ray->perp_wall_dist);

    ray->draw_start = -ray->line_height / 2 + WIN_HEIGHT / 2;
    if (ray->draw_start < 0) ray->draw_start = 0;

    ray->draw_end = ray->line_height / 2 + WIN_HEIGHT / 2;
    if (ray->draw_end >= WIN_HEIGHT) ray->draw_end = WIN_HEIGHT - 1;
}

/* pick_texture now only writes to ray->tex_num (ok) and optionally set ray->tex pointer */
void pick_texture(t_game *game, t_ray *ray)
{
    if (ray->side == 0 && ray->ray_dir_x > 0)
        ray->tex_num = 3; /* east */
    else if (ray->side == 0 && ray->ray_dir_x < 0)
        ray->tex_num = 2; /* west */
    else if (ray->side == 1 && ray->ray_dir_y > 0)
        ray->tex_num = 1; /* north */
    else
        ray->tex_num = 0; /* south */

    ray->tex = &game->textures[ray->tex_num].img;
}

/* texture_and_coordinate fills ray->wall_x, tex_x, step, tex_pos */
void texture_and_coordinate(t_game *game, t_ray *ray)
{
    if (!ray->tex || ray->tex->width <= 0 || ray->tex->height <= 0 || !ray->tex->addr)
        return;

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

	/* how many steps of the pixel texture needed to cover the wall height, example if the texture is 50px and the screen is 100px, the step is 0.5*/
    ray->step = 1.0 * ray->tex->height / (double)ray->line_height;
	/* compute where the position of the texture will start at the top (if its shrinked or expanded)*/
    ray->tex_pos = (ray->draw_start - WIN_HEIGHT / 2.0 + ray->line_height / 2.0) * ray->step;
}

/* draw_vertical_line uses ray->tex, ray->tex_pos, ray->step etc. */
void draw_vertical_line(t_game *game, t_ray *ray, int x)
{
    if (!ray->tex || !ray->tex->addr)
        return;

    for (int y = ray->draw_start; y < ray->draw_end; y++)
    {
        int tex_y = (int)ray->tex_pos;
        if (tex_y < 0) tex_y = 0;
        if (tex_y >= ray->tex->height) tex_y = ray->tex->height - 1;
        ray->tex_pos += ray->step;

        char *tex_dst = ray->tex->addr + (tex_y * ray->tex->line_length
                            + ray->tex_x * (ray->tex->bits_per_pixel / 8));
        unsigned int color = *(unsigned int *)tex_dst;

        char *dst = game->img.addr + (y * game->img.line_length
                            + x * (game->img.bits_per_pixel / 8));
        *(unsigned int *)dst = color;
    }
}


void    render_walls(t_game *game)
{
	t_ray ray;
    for (int x = 0; x < WIN_WIDTH; x++)
    {
		ray_setup(game, &ray, x);
        // --- 2. DDA loop ---
        dda_loop(game, &ray);
        // --- 3. Distance to wall ---
        distance_to_wall(game, &ray);
        // --- 4. Pick texture ---
        pick_texture(game, &ray);
        // --- 5. Texture X coordinate ---
        texture_and_coordinate(game, &ray);
        // --- 7. Draw vertical stripe ---
        draw_vertical_line(game, &ray, x);
    }
}


// --- Setup player defaults ---
void	init_player(t_player *p)
{
	p->pos_x = 3.5;   // player position (x)
	p->pos_y = 3.5;   // player position (y)
	p->dir_x = 1.0;  // facing left (west)
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
    if (game->map[(int)(game->player.pos_y)][(int)(game->player.pos_x + game->player.dir_x * MOVE_SPEED)] == '0')
        game->player.pos_x += game->player.dir_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y + game->player.dir_y * MOVE_SPEED)][(int)(game->player.pos_x)] == '0')
        game->player.pos_y += game->player.dir_y * MOVE_SPEED;
}

void move_backward(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)][(int)(game->player.pos_x - game->player.dir_x * MOVE_SPEED)] == '0')
        game->player.pos_x -= game->player.dir_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y - game->player.dir_y * MOVE_SPEED)][(int)(game->player.pos_x)] == '0')
        game->player.pos_y -= game->player.dir_y * MOVE_SPEED;
}

void strafe_left(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)][(int)(game->player.pos_x - game->player.plane_x * MOVE_SPEED)] == '0')
        game->player.pos_x -= game->player.plane_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y - game->player.plane_y * MOVE_SPEED)][(int)(game->player.pos_x)] == '0')
        game->player.pos_y -= game->player.plane_y * MOVE_SPEED;
}

void strafe_right(t_game *game)
{
    if (game->map[(int)(game->player.pos_y)][(int)(game->player.pos_x + game->player.plane_x * MOVE_SPEED)] == '0')
        game->player.pos_x += game->player.plane_x * MOVE_SPEED;
    if (game->map[(int)(game->player.pos_y + game->player.plane_y * MOVE_SPEED)][(int)(game->player.pos_x)] == '0')
        game->player.pos_y += game->player.plane_y * MOVE_SPEED;
}


void rotate_left(t_game *game)
{
    double old_dir_x = game->player.dir_x;
    game->player.dir_x = game->player.dir_x * cos(ROT_SPEED) - game->player.dir_y * sin(ROT_SPEED);
    game->player.dir_y = old_dir_x * sin(ROT_SPEED) + game->player.dir_y * cos(ROT_SPEED);

    double old_plane_x = game->player.plane_x;
    game->player.plane_x = game->player.plane_x * cos(ROT_SPEED) - game->player.plane_y * sin(ROT_SPEED);
    game->player.plane_y = old_plane_x * sin(ROT_SPEED) + game->player.plane_y * cos(ROT_SPEED);
}

void rotate_right(t_game *game)
{
    double old_dir_x = game->player.dir_x;
    game->player.dir_x = game->player.dir_x * cos(-ROT_SPEED) - game->player.dir_y * sin(-ROT_SPEED);
    game->player.dir_y = old_dir_x * sin(-ROT_SPEED) + game->player.dir_y * cos(-ROT_SPEED);

    double old_plane_x = game->player.plane_x;
    game->player.plane_x = game->player.plane_x * cos(-ROT_SPEED) - game->player.plane_y * sin(-ROT_SPEED);
    game->player.plane_y = old_plane_x * sin(-ROT_SPEED) + game->player.plane_y * cos(-ROT_SPEED);
}

int key_press(int keycode, t_game *game)
{
    if (keycode >= 0 && keycode < 256)
        game->keys[keycode] = 1;

    if (keycode == KEY_LEFT)
        game->key_left = 1;
    if (keycode == KEY_RIGHT)
        game->key_right = 1;
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


// --- Rendering (empty for now, you’ll add raycasting later) ---
int render_frame(t_game *game)
{
    /* movement keys (W/A/S/D are ASCII < 256 and safe to index) */
    if (game->keys[KEY_W])    move_forward(game);
    if (game->keys[KEY_S])    move_backward(game);
    if (game->keys[KEY_A])    strafe_left(game);
    if (game->keys[KEY_D])    strafe_right(game);

    /* use explicit flags for arrows (you already set these in key_press/key_release) */
    if (game->key_left)       rotate_right(game);
    if (game->key_right)      rotate_left(game);

    /* handle ESC on key press (avoid indexing keys[KEY_ESC] which is >256) */
    /* if you prefer a flag for ESC, set it in key_press like key_left/right */

    /* render */
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
