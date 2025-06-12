# Makefile for modular SDL2 tank game (macOS / Linux ARM64)

CC = clang
CFLAGS = -Wall -Wextra -std=c11 -O2 `sdl2-config --cflags` -I.
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

TARGET = tank_game
SRCS = mount_system.c main.c tank.c car.c entity.c exhaust_flame.c
OBJS = mount_system.o main.o tank.o car.o entity.o exhaust_flame.o
HDRS = mount_system.h entity.h tank.h car.h exhaust_flame.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
