#include "cub3d.h"

// --- Close window ---
int	close_window(t_game *game)
{
	if (game->win)
		mlx_destroy_window(game->mlx, game->win);
	exit(0);
	return (0);
}

// --- Setup player defaults ---
void	init_player(t_player *p) /*mad*/
{
	p->pos_x = 3.5;   // player position (x)
	p->pos_y = 3.5;   // player position (y)
	p->dir_x = 1.0;  // facing left (east)
	p->dir_y = 0.0;
	p->plane_x = 0;
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
	ft_memset(game->keys, 0, sizeof(game->keys));
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
		printf("Error: failed to load texture %s\n", path);
		exit(1);                                               //////////check exit fcuntion
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
	game->floor_color = 3355443;   // dark gray
	game->ceiling_color = 8900331; // sky blue
}

// --- Default test map ---
char *default_map[] = {
	"1111111111",
	"1000000101",
	"1000110001",
	"1100000101",
	"1000100111",
	"1000000101",
	"1111111111",
	NULL
};

// --- Attach default map to game ---
void	init_map(t_game *game)
{
	game->map = default_map;
	game->map_width = ft_strlen(default_map[0]);
	int h = 0;
	while (default_map[h])
		h++;
	game->map_height = h;
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
		return (printf("Error\nmlx_init failed\n"), 1);

	// Create window
	game.win = mlx_new_window(game.mlx, WIN_WIDTH, WIN_HEIGHT, "cub3D");
	if (!game.win)
		return (printf("Error\nmlx_new_window failed\n"), 1);

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
