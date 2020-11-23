CFLAGS=-Wall -g -Wextra -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
CC=gcc

LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

#BUILD=build
SRC=src

SOURCES=$(wildcard $(SRC)/**/*.c $(SRC)/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))


TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c, %, $(TEST_SRC))

NOTIF_SOURCES=$(wildcard src/**/notif.c, src/notif.c)
NOTIF_OBJECTS=$(patsubst %.c,%.o, $(NOTIF_SOURCES))


notif: $(NOTIF_OBJECTS)
	$(CC) -o $(BUILD)/notif $(NOTIF_OBJECTS)

run_notif:
	build/notif

TARGET=build/lib.a
# NOTE: Specify your own name with: make all TARGET=build/your_own_lib.a
S0_TARGET=$(patsubst %.a, %.so, $(TARGET))

# The target build
all: $(TARGET) $(SO_TARGET) tests

dev: CFLAGS=-g -Wall -Isrc -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)

build:
	@mkdir -p build
	@mkdir -p bin

# Unit Tests
.PHONY: tests
tests: LDLIBS += $(TARGET)
tests: $(TESTS) valgrind
	sh ./tests/runtests.sh

valgrind:
	VALGRIND="valgrind --tool=memcheck --log-file=/tmp/valgrind-%p.log"


clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -rf tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
