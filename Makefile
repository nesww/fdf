FLAGS = -Wall -Wextra -Werror
NAME = fdf

BUILD_DIR = build/
INCLUDE_DIR = include/
SOURCE_DIR = src/

MLX_FLAGS = -lmlx42 -lglfw -pthread -lm -ldl

all: clean build

clean:
	rm -rf $(BUILD_DIR)$(NAME)

build: $(INCLUDE_DIR)* $(SOURCE_DIR)$(NAME).c
	mkdir -p $(BUILD_DIR)
	gcc $(FLAGS) -g $(MLX_INCLUDE) -o $(BUILD_DIR)$(NAME) $(SOURCE_DIR)$(NAME).c $(MLX_FLAGS)
	
include:
	sudo cp $(INCLUDE_DIR)$(NAME).h /usr/local/include


.PHONY: all clean build debug
