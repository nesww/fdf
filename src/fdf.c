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

#define WIN_WIDTH 1280
#define WIN_HEIGHT 1080
#define DIST_SCALE 5 // px
#define COLOR_SCALE 20

#define RED 0xff0000ff
#define BLUE 0x0000ffff
#define GREEN 0x00ff00ff
#define WHITE 0xffffffff
#define BLACK 0x000000ff

typedef struct vec2_s {
  i32 x;
  i32 y;
} vec2;

typedef struct color_s {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} Color;

typedef struct fdfmap_s {
  i32 **buf;
  u32 len;
  u32 width;
} fdfmap_t;

u32 color_to_hex(Color *c) {
  u32 hex = 0x0;
  hex = (hex << 8) + c->r;
  hex = (hex << 8) + c->g;
  hex = (hex << 8) + c->b;
  hex = (hex << 8) + c->a;
  return hex;
}

u32 ft_abs(i32 n) { return n < 0 ? -n : n; }

u32 euclidian_dist_sq(vec2 *src, vec2 *dst) {
  i32 dx = src->x - dst->x;
  i32 dy = src->y - dst->y;
  return ft_abs(dx * dx + dy * dy);
}

void draw_circle(mlx_image_t *img, vec2 *center_pos, u32 radius, Color *color) {
  u32 c = color_to_hex(color);
  for (u32 x = center_pos->x - radius; x < center_pos->x + radius; ++x) {
    for (u32 y = center_pos->y - radius; y < center_pos->y + radius; ++y) {
      vec2 p = {.x = x, .y = y};
      if (euclidian_dist_sq(center_pos, &p) < radius * radius) {
        mlx_put_pixel(img, x, y, c);
      }
    }
  }
}

void draw_square(mlx_image_t *img, vec2 *center_pos, u32 radius, Color *color) {
  // io_printf("will draw at x:%d y:%d with a radius of %d\n", center_pos->x,
  //          center_pos->y, radius);
  u32 c = color_to_hex(color);
  for (u32 x = center_pos->x - radius; x < center_pos->x + radius; ++x) {
    for (u32 y = center_pos->y - radius; y < center_pos->y + radius; ++y) {
      mlx_put_pixel(img, x, y, c);
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

u32 buf_len(i32 **buf) {
  u32 k = 0;
  while (buf[k])
    ++k;
  return k;
}

fdfmap_t *load_fdf(int fd, char *fdf_path) {
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
  u32 word_count;
  io_printf("loading fdf from `%s` into memory...", fdf_path);
  while (line) {

    word_count = 0;
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
  fdfmap_t *fdf = malloc(sizeof(fdfmap_t));
  fdf->buf = buf;
  fdf->len = buf_len(fdf->buf);
  fdf->width = word_count - 1;

  io_printf("success\n");
  return fdf;
}

u8 normalize_hex(u32 hex) {
  if (hex > 0xff) {
    return 0xff;
  }
  return hex;
}

void draw_buf(mlx_image_t *img, fdfmap_t *fdf, u32 offset) {

  io_printf("draw_buf called with offset= %d\n", offset);
  io_printf("fdf->len: %d\n", fdf->len);
  io_printf("fdf->width: %d\n", fdf->width);

  vec2 start = {(WIN_WIDTH - (fdf->width * offset)) / 2,
                (WIN_HEIGHT - (fdf->len * offset)) / 2};

  if (start.x < 0 || start.x > WIN_WIDTH) {
    start.x = 0;
    offset = 2;
  }
  if (start.y < 0 || start.y > WIN_WIDTH) {
    start.y = 0;
    offset = 2;
  }

  io_printf("start(%d, %d)\n", start.x, start.y);

  for (u32 i = 0; i < fdf->len; ++i) {
    u32 j = 0;
    while (fdf->buf[i][j] != INT_MAX) {
      vec2 pos = {start.x + j * offset, start.y + i * offset};
      // io_printf("pos(%d,%d)\n", pos.x, pos.y);
      if (pos.x < 0 || pos.x >= (i32)WIN_WIDTH || pos.y < 0 ||
          pos.y >= (i32)WIN_HEIGHT) {
        // io_printf("warning: computed pos(%d,%d), out of image, skipping\n",
        //          pos.x, pos.y);
        ++j;
        continue;
      }

      // if (i % 5 && j % 5) {
      Color c = {normalize_hex(0x66 + fdf->buf[i][j] * COLOR_SCALE),
                 normalize_hex(0x88 + fdf->buf[i][j] * COLOR_SCALE),
                 normalize_hex(0x33 + fdf->buf[i][j] * COLOR_SCALE), 0xff};

      // io_printf("Color: (%x, %x, %x, %x)\n", c.r, c.g, c.b, c.a);
      draw_square(img, &pos, offset / 1.8, &c);
      //}
      ++j;
    }
  }
}

typedef struct iso_point_s {
  vec2 *pos;
  vec2 *east_conn;
  vec2 *south_conn;
} iso_point_t;

// iso_point_t rec_iso_point_neighbors_of(vec2 *pos, fdfmap_t *fdf) {

// stop condition -> found bottom_right corner has no south AND no east
//  if (pos->x + 1 >= fdf->width && pos->y + 1 >= fdf->len) {
//    return (iso_point_t){pos, NULL, NULL};
//  }

// right border, no east, but south
// if (pos->x + 1 >= fdf->width && !(pos->y + 1 >= fdf->len)) {
//  vec2 tmp = {pos->x, pos->y + 1};
// iso_point_t south = rec_iso_point_neighbors_of(&tmp, fdf);
// return (iso_point_t){pos, NULL, south.pos};
// }
// bottom border, no south but east
// else if (pos->y >= fdf->len) {
// vec2 tmp = {pos->x + 1, pos->y};
//  iso_point_t east = rec_iso_point_neighbors_of(&tmp, fdf);
//  return (iso_point_t){pos, east.pos, NULL};
// }
// has both east and south
// else {
// vec2 tmp_east = {pos->x + 1, pos->y};
// vec2 tmp_south = {pos->x, pos->y + 1};
// iso_point_t east = rec_iso_point_neighbors_of(&tmp_east, fdf);
// iso_point_t south = rec_iso_point_neighbors_of(&tmp_south, fdf);
// return (iso_point_t){pos, east.pos, south.pos};
//}
//}

void clean_iso_points(iso_point_t *buf, u32 cursor) {
  for (u32 i = 0; i < cursor; ++i) {
    free(buf[i].pos);
    free(buf[i].east_conn);
    free(buf[i].south_conn);
  }
  free(buf);
}

// get all isometrics positions for all points of ftmap_t
iso_point_t *iso_points(fdfmap_t *fdf, u32 offset_x, u32 offset_y, vec2 *origin,
                        i32 height_scale) {

  iso_point_t *points = malloc(sizeof(iso_point_t) * (fdf->len * fdf->width));
  if (!points)
    return NULL;
  u32 points_cursor = 0;
  for (u32 y = 0; y < fdf->len; ++y) {
    for (u32 x = 0; x < fdf->width; ++x) {

      vec2 *pos = malloc(sizeof(vec2));
      if (!pos) {
        clean_iso_points(points, points_cursor);
        return NULL;
      }
      pos->x = (x - y) * offset_x + origin->x;
      pos->y = (x + y) * offset_y - fdf->buf[y][x] * height_scale + origin->y;

      vec2 *east_conn;
      if (x + 1 >= fdf->width) {
        east_conn = NULL;
      } else {
        east_conn = malloc(sizeof(vec2));
        if (!east_conn) {
          clean_iso_points(points, points_cursor);
          free(points);
          return NULL;
        }
        east_conn->x = ((x + 1) - y) * offset_x + origin->x;
        east_conn->y = ((x + 1) + y) * offset_y -
                       fdf->buf[y][x + 1] * height_scale + origin->y;
      }

      vec2 *south_conn;
      if (y + 1 >= fdf->len) {
        south_conn = NULL;
      } else {
        south_conn = malloc(sizeof(vec2));
        if (!south_conn) {
          clean_iso_points(points, points_cursor);
          return NULL;
        }
        south_conn->x = (x - (y + 1)) * offset_x + origin->x;
        south_conn->y = (x + (y + 1)) * offset_y -
                        fdf->buf[y + 1][x] * height_scale + origin->y;
      }

      iso_point_t p = {
          .pos = pos, .east_conn = east_conn, .south_conn = south_conn};

      // io_printf("initial pos: pos(%d,%d,%d) of fdf\n", x, y, fdf->buf[y][x]);
      points[points_cursor] = p;
      ++points_cursor;
    }
  }
  return points;
}

typedef struct s_vars {
  mlx_t *mlx;
  mlx_image_t *img;
  fdfmap_t *fdf;
  int height_scale;
  u32 offset_x;
  u32 offset_y;
  vec2 *origin;
} vars_t;

void redraw(vars_t *vars) {
  iso_point_t *points = iso_points(vars->fdf, vars->offset_x, vars->offset_y,
                                   vars->origin, vars->height_scale);
  clear_image(vars->img, 0x3333333f);

  io_printf("redrawing!\n");

  for (u32 i = 0; i < vars->fdf->width * vars->fdf->len; ++i) {
    if (points[i].pos->x < 0 || points[i].pos->y < 0 ||
        points[i].pos->x >= WIN_WIDTH || points[i].pos->y >= WIN_HEIGHT) {
      io_printf("skipping point of pos(%d,%d) -> out of bound\n",
                points[i].pos->x, points[i].pos->y);
      continue;
    }
    Color red = {0xff, 0x00, 0x00, 0xff};
    draw_circle(vars->img, points[i].pos, 2, &red);
    if (points[i].east_conn)
      draw_line(vars->img, points[i].pos, points[i].east_conn, 0xffffffff);
    if (points[i].south_conn)
      draw_line(vars->img, points[i].pos, points[i].south_conn, 0xffffffff);
  }
}

void key_handler(mlx_key_data_t keydata, void *param) {
  vars_t *vars = (vars_t *)param;
  if (keydata.key == MLX_KEY_UP && keydata.action == MLX_PRESS) {
    io_printf("key_up event!\n");
    if (vars->height_scale + 1 < 30)
      vars->height_scale += 1;
    redraw(vars);
  } else if (keydata.key == MLX_KEY_DOWN && keydata.action == MLX_PRESS) {
    io_printf("key_down event!\n");
    if (vars->height_scale - 1 > -10)
      vars->height_scale -= 1;
    redraw(vars);
  } else if (keydata.key == MLX_KEY_ESCAPE && keydata.action == MLX_PRESS) {
    mlx_close_window(vars->mlx);
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
  fdfmap_t *fdf = load_fdf(fdf_file, fdf_path);
  if (!fdf) {
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

  // offset for scaling in the window
  i32 h_offset = WIN_WIDTH / fdf->width;
  i32 v_offset = WIN_HEIGHT / fdf->len;
  i32 offset = h_offset > v_offset ? v_offset : h_offset;
  if (offset < 5)
    offset = 5;

  // draw_buf(img, fdf, offset);
  vec2 iso_center = {WIN_WIDTH / 2, WIN_HEIGHT / 3};
  i32 height_scale = 1;
  iso_point_t *points =
      iso_points(fdf, h_offset / 4, v_offset / 4, &iso_center, height_scale);

  // draw points
  for (u32 i = 0; i < fdf->width * fdf->len; ++i) {
    if (points[i].pos->x < 0 || points[i].pos->y < 0 ||
        points[i].pos->x >= WIN_WIDTH || points[i].pos->y >= WIN_HEIGHT) {
      io_printf("skipping point of pos(%d,%d) -> out of bound\n",
                points[i].pos->x, points[i].pos->y);
      continue;
    }
    Color red = {0xff, 0x00, 0x00, 0xff};
    draw_circle(img, points[i].pos, 2, &red);
    if (points[i].east_conn)
      draw_line(img, points[i].pos, points[i].east_conn, 0xffffffff);
    if (points[i].south_conn)
      draw_line(img, points[i].pos, points[i].south_conn, 0xffffffff);
  }
  free(points);

  io_printf("starting mlx loop\n");
  vars_t vars = {mlx,          img,          fdf,        height_scale,
                 h_offset / 4, v_offset / 4, &iso_center};

  mlx_key_hook(mlx, key_handler, (void *)&vars);

  mlx_loop(mlx);

  free_buf(fdf->buf, fdf->len);
  io_printf("closing...\n");
  mlx_terminate(mlx);
  return 0;
}
