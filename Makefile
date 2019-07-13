CC = clang
CFLAGS = -O3 -mtune=znver1 -march=core2 -pipe -g3

OUTPUT = freedomedit

SOURCES = main.c syntax.c

all:
	$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` $(SOURCES) -o $(BUILD_PREFIX)freedomedit `pkg-config --libs gtk+-3.0`

clean:
	rm -rf $(BUILD_PREFIX)freedomedit
	rm -rf $(BUILD_PREFIX)*.o
