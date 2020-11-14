CFLAGS=-Wall -g -Wextra -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
CC=gcc

SOURCES=$(wildcard $(SRC)/**/*.c $(SRC)/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

BUILD=build
SRC=src

NOTIF_SOURCES=$(wildcard src/**/notif.c, src/notif.c)
NOTIF_OBJECTS=$(patsubst %.c,%.o, $(NOTIF_SOURCES))


notif: $(NOTIF_OBJECTS)
	$(CC) -o $(BUILD)/notif $(NOTIF_OBJECTS)

run_notif:
	build/notif

clean:
	rm -f $(SRC)/*.o
	rm -f $(BUILD)/*
