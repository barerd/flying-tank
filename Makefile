# Makefile for modular SDL2 tank game (macOS / Linux ARM64)

CC = clang
CFLAGS = -Wall -Wextra -std=c11 -O2 `sdl2-config --cflags` `pkg-config --cflags libcjson` -I.
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf `pkg-config --libs libcjson`

TARGET = tank_game
SRCS = mount_system.c main.c entity.c entity_spawn_animated.c entity_render_helpers.c behavior_helpers.c sdl_helpers.c mount_helpers.c bullet.c collision.c hitbox_loader.c
OBJS = $(SRCS:.c=.o)
HDRS = mount_system.h entity.h entity_spawn_animated.h entity_render_helpers.h behavior_helpers.h sdl_helpers.h mount_helpers.h bullet.h collision.h hitbox_loader.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
