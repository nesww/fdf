#include <MLX42/MLX42.h>
#include <fdf.h>
#include <libft/libft.h>
#include <stdio.h>
#include <unistd.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 800

void ft_hook(void *p) {
  const mlx_t *mlx = p;

  printf("w: %d | h: %d\n", mlx->width, mlx->height);
}

int main(void) {
  mlx_t *mlx = mlx_init(WIN_WIDTH, WIN_HEIGHT, "fdf", 0);
  if (!mlx)
    return 1;

  mlx_image_t *img = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
  if (!img || (mlx_image_to_window(mlx, img, 0, 0)) < 0)
    return 1;

  for (u32 i = 100; i < 300; ++i) {
    for (u32 j = 100; j < 300; ++j) {
      mlx_put_pixel(img, i, j, 0xFF0000AA);
    }
  }
  mlx_loop_hook(mlx, ft_hook, mlx);
  mlx_loop(mlx);
  mlx_terminate(mlx);
  return 0;
}
