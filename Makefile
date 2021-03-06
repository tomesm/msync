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

TARGET=build/lib.a
# NOTE: Specify your own name with: make all TARGET=build/your_own_lib.a
S0_TARGET=$(patsubst %.a, %.so, $(TARGET))

client:  
	$(CC) -g -o client client.c $(TARGET)

server:
	$(CC) -g -o server server.c $(TARGET)

# Run debug server to deal with memory leaks
dserver:
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes ./server

# Run debug client to deal with memory leaks
dclient:
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes ./client dirtest

# The target build
all: $(TARGET) $(SO_TARGET) tests client server

dev: CFLAGS=-g -Wall -Isrc -Wextra -pedantic -Werror $(OPTFLAGS)
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
	VALGRIND="valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes --log-file=/tmp/valgrind-%p.log"


clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -rf tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`
	rm -rf src/*.o
	rm -rf server
	rm -rf client
