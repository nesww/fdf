#include <fdf.h>
#include <libft/libft.h>
#include <mlx.h>
#include <unistd.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 800

typedef struct vars_s {
  void *win;
  void *mlx;
} vars_t;

typedef struct data_s {
  void *img;
  char *addr;
  i32 bits_per_pixel;
  i32 line_length;
  i32 endian;
} data_t;

u32 get_offset(data_t *img, u32 x, u32 y) {
  return (y * img->line_length + x * (img->bits_per_pixel / 8));
}

void put_pixel(data_t *buf, u32 x, u32 y, u32 color) {
  char *dst;

  if (x >= WIN_WIDTH || y >= WIN_HEIGHT)
    return;

  dst = buf->addr + get_offset(buf, x, y);
  *(unsigned int *)dst = color;
}

int main(void) {
  vars_t vars;
  vars.mlx = mlx_init();
  vars.win = mlx_new_window(vars.mlx, WIN_WIDTH, WIN_HEIGHT, "fdf");

  data_t img;
  img.img = mlx_new_image(vars.mlx, WIN_WIDTH, WIN_HEIGHT);

  img.addr = mlx_get_data_addr(img.img, &img.bits_per_pixel, &img.line_length,
                               &img.endian);

  for (u32 i = 100; i < 500; ++i) {
    for (u32 j = 100; j < 500; ++j) {
      put_pixel(&img, i, j, 0x00FF0000);
    }
  }
  mlx_put_image_to_window(vars.mlx, vars.win, img.img, 0, 0);
  mlx_loop(vars.mlx);

  mlx_destroy_window(vars.mlx, vars.win);
  return 0;
}
