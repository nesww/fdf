FLAGS = -Wall -Wextra -Werror
NAME = fdf

BUILD_DIR = build/
INCLUDE_DIR = include/
SOURCE_DIR = src/
MLX_DIR = lib/minilibx-linux

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    MLX_FLAGS = -L$(MLX_DIR) -lmlx -framework OpenGL -framework AppKit
    MLX_INCLUDE = -I$(MLX_DIR)
    EXTRA_FLAGS = 
else ifeq ($(UNAME_S),Linux)
    MLX_FLAGS = -L$(MLX_DIR) -lmlx -lXext -lX11 -lm -lz
    MLX_INCLUDE = -I$(MLX_DIR)
    EXTRA_FLAGS = 
endif

all: clean build

clean:
	rm -rf $(BUILD_DIR)$(NAME)

$(MLX_DIR)/libmlx.a:
	make -C $(MLX_DIR)

build: $(MLX_DIR)/libmlx.a $(INCLUDE_DIR)* $(SOURCE_DIR)$(NAME).c
	mkdir -p $(BUILD_DIR)
	sudo cp $(INCLUDE_DIR)$(NAME).h /usr/local/include
	gcc $(FLAGS) -g $(MLX_INCLUDE) -o $(BUILD_DIR)$(NAME) $(SOURCE_DIR)$(NAME).c $(MLX_FLAGS)

debug:
	@echo "OS détecté: $(UNAME_S)"
	@echo "Flags MLX: $(MLX_FLAGS)"
	@echo "Include MLX: $(MLX_INCLUDE)"

.PHONY: all clean build debug

