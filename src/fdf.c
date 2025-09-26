#include <MLX42/MLX42.h>
#include <fcntl.h>
#include <fdf.h>
#include <gnl/get_next_line.h>
#include <libft/ftio/ftio.h>
#include <libft/ftypes.h>
#include <libft/libft.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h> //for free/malloc
#include <unistd.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define DIST_SCALE 50 // px

#define RED 0xff0000ff
#define BLUE 0x0000ffff
#define GREEN 0x00ff00ff
#define WHITE 0xffffffff
#define BLACK 0x000000ff

typedef struct vec2_s {
  i32 x;
  i32 y;
} vec2;

u32 ft_abs(i32 n) { return n < 0 ? -n : n; }

u32 euclidian_dist_sq(vec2 *src, vec2 *dst) {
  i32 dx = src->x - dst->x;
  i32 dy = src->y - dst->y;
  return ft_abs(dx * dx + dy * dy);
}

void draw_circle(mlx_image_t *img, vec2 *center_pos, u32 radius, u32 color) {
  for (u32 x = center_pos->x - radius; x < center_pos->x + radius; ++x) {
    for (u32 y = center_pos->y - radius; y < center_pos->y + radius; ++y) {
      vec2 p = {.x = x, .y = y};
      if (euclidian_dist_sq(center_pos, &p) < radius * radius) {
        mlx_put_pixel(img, x, y, color);
      }
    }
  }
}

void draw_line(mlx_image_t *img, vec2 *src, vec2 *dst, u32 color) {
  i32 dx = dst->x - src->x;
  i32 dy = dst->y - src->y;

  i32 incX = dx < 0 ? -1 : 1;
  i32 incY = dy < 0 ? -1 : 1;
  u32 e = 0;
  vec2 cursor = {src->x, src->y};
  // move by x
  if (ft_abs(dx) >= ft_abs(dy)) {
    for (; cursor.x != dst->x; cursor.x += incX) {
      mlx_put_pixel(img, cursor.x, cursor.y, color);
      e += ft_abs(dy);
      if (e >= ft_abs(dx)) {
        e -= ft_abs(dx);
        cursor.y += incY;
      }
    }
  }
  // move by y
  else {
    for (; cursor.y != dst->y; cursor.y += incY) {
      mlx_put_pixel(img, cursor.x, cursor.y, color);
      e += ft_abs(dx);
      if (e >= ft_abs(dy)) {
        e -= ft_abs(dy);
        cursor.x += incX;
      }
    }
  }
}

void free_buf(i32 **buf, u32 len) {
  for (u32 j = 0; j < len; ++j) {
    if (buf[j])
      free(buf[j]);
  }
  free(buf);
}

vec2 rotate_line(vec2 *src, vec2 *dst, double angle) {
  double rad = angle * M_PI / 180;
  i32 u = dst->x - src->x;
  i32 v = dst->y - src->y;

  i32 du = u * cos(rad) - v * sin(rad);
  i32 dv = u * sin(rad) + v * cos(rad);

  vec2 q = {src->x + du, src->y + dv};
  return q;
}

void clear_image(mlx_image_t *img, u32 color) {
  for (u32 i = 0; i < img->width; ++i) {
    for (u32 j = 0; j < img->height; ++j) {
      mlx_put_pixel(img, i, j, color);
    }
  }
}

i32 **load_fdf(int fd, char *fdf_path) {
  i32 **buf = malloc(sizeof(i32 *) * 4096);
  if (!buf) {
    io_printf("error: could not malloc for buf of fdf\n");
    return NULL;
  }
  char *line = get_next_line(fd);
  if (!line) {
    io_printf("error: initial line was NULL from gnl\n");
    return NULL;
  }

  u32 i = 0;
  io_printf("loading fdf from `%s` into memory...", fdf_path);
  while (line) {

    u32 word_count = 0;
    char **split = ft_split(line, ' ');
    while (split[word_count])
      ++word_count;

    buf[i] = malloc(sizeof(i32) * (word_count) + 1);
    for (u32 j = 0; j < word_count; ++j) {
      buf[i][j] = ft_atoi(split[j]);
      free(split[j]);
    }
    buf[i][word_count] = INT_MAX;
    free(split);
    free(line);

    line = get_next_line(fd);
    ++i;
  }
  io_printf("success\n");
  return buf;
}

u32 buf_len(i32 **buf) {
  u32 k = 0;
  while (buf[k])
    ++k;
  return k;
}

void draw_buf(mlx_image_t *img, i32 **buf, u32 offset) {
  u32 len = buf_len(buf);
  io_printf("len= %d\n", len);
  for (u32 i = 0; i < len; ++i) {
    u32 j = 0;
    while (buf[i][j]) {
      io_printf("drawing x:%d y:%d, given offset=%d (x:%d y: %d)\n", i, j,
                offset, i + i * offset, j + j * offset);
      vec2 pos = {i + i * offset, j + j * offset};
      draw_circle(img, &pos, 3, 0xff0000aa);
      ++j;
    }
    io_printf("j=%d after one loop (of i=%d)\n", j, i);
  }
}
int main(int argc, char **argv) {
  io_printf("Startup of fdf...\n");
  // load  of file from args
  if (argc < 2) {
    io_printf("error: missing fdf file as argument, exiting...\n");
    return 1;
  }

  char *fdf_path = argv[1];
  int fdf_file = open(fdf_path, O_RDONLY);
  if (fdf_file < 0) {
    io_printf(
        "error: fdf file path provided does not exist, check path: `%s`\n",
        fdf_path);
    return 1;
  }

  // load fdf into a buffer
  i32 **buf = load_fdf(fdf_file, fdf_path);
  if (!buf) {
    io_printf("fatal: failed to load `%s` into memory, exiting\n");
  }
  io_printf("closing fd\n", fdf_path);
  close(fdf_file);

  //////////////////////
  /// graphical part ///
  //////////////////////

  // start window
  io_printf("initializing mlx_window...");
  mlx_t *mlx = mlx_init(WIN_WIDTH, WIN_HEIGHT, "fdf", 0);
  if (!mlx)
    return 1;
  io_printf("success\n");

  mlx_image_t *img = mlx_new_image(mlx, WIN_WIDTH, WIN_HEIGHT);
  if (!img || (mlx_image_to_window(mlx, img, 0, 0)) < 0)
    return 1;

  // vec2 src = {.x = WIN_WIDTH / 2, .y = WIN_HEIGHT / 2};

  // vec2 left = {src.x - 100, src.y};
  // vec2 right = {src.x + 100, src.y};

  // vec2 rotated_left = rotate_line(&src, &left, 30);
  // vec2 rotated_right = rotate_line(&src, &right, -30);

  // clear_image(img, BLACK);

  // draw_line(img, &src, &left, BLUE);
  // draw_line(img, &src, &right, GREEN);
  // draw_line(img, &src, &rotated_left, GREEN);
  // draw_line(img, &src, &rotated_right, BLUE);
  // draw_line(img, &rotated_left, &rotated_right, 0xff000077);
  // draw_circle(img, &src, 5, RED);
  draw_buf(img, buf, DIST_SCALE);
  io_printf("starting mlx loop\n");
  mlx_loop(mlx);
  free_buf(buf, buf_len(buf));
  io_printf("closing...\n");
  mlx_terminate(mlx);
  return 0;
}
