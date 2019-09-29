CC = clang
CFLAGS = -O3 -mtune=znver1 -march=core2 -pipe -g3 -std=c99 -I./inc

OUTPUT = freedomedit

SOURCES = main.c syntax/syntax.c

all:
	$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0 gtksourceview-4` \
	$(SOURCES) -o $(BUILD_PREFIX)freedomedit \
	`pkg-config --libs gtk+-3.0 gtksourceview-4` 

clean:
	rm -rf $(BUILD_PREFIX)freedomedit
	rm -rf $(BUILD_PREFIX)*.o
