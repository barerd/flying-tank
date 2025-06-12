# Makefile for modular SDL2 tank game (macOS / Linux ARM64)

CC = clang
CFLAGS = -Wall -Wextra -std=c11 -O2 `sdl2-config --cflags` -I.
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf

TARGET = tank_game
SRCS = main.c tank.c car.c entity.c exhaust_flame.c debug_text.c
OBJS = main.o tank.o car.o entity.o exhaust_flame.o debug_text.o
HDRS = entity.h tank.h car.h exhaust_flame.h debug_text.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
