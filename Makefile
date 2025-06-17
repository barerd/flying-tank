# Makefile for modular SDL2 tank game (macOS / Linux ARM64)

CC = clang
CFLAGS = -Wall -Wextra -std=c11 -O2 `sdl2-config --cflags` -I.
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

TARGET = tank_game
SRCS = mount_system.c main.c entity.c entity_spawn_animated.c entity_render_helpers.c behavior_helpers.c sdl_helpers.c mount_helpers.c bullet.c
OBJS = mount_system.o main.o entity.o entity_spawn_animated.o entity_render_helpers.o behavior_helpers.o sdl_helpers.o mount_helpers.o bullet.o
HDRS = mount_system.h entity.h entity_spawn_animated.h entity_render_helpers.h behavior_helpers.h sdl_helpers.c mount_helpers.h bullet.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
