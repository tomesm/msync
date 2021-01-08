cmake_minimum_required(VERSION 3.19.2)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(msync C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_CFLAGS "-Wall -g -Wextra -Isrc -rdynamic -DNDEBUG")

set(LIBS "-ldl")

set(SRC "src")
set(TEST_SRC "tests")
# set(SOURCES ${wildcard ${SRC}/**/*.c ${SRC}/*.c)

set(LINUX_SOURCES
    ${SRC}/bstrlib.c
    ${SRC}/collections.c
    ${SRC}/fs.c
    ${SRC}/net.c 
    ${SRC}/notify.c
)

set(LINUX_HEADERS
    ${SRC}/bstrlib.h 
    ${SRC}/collections.h 
    ${SRC}/debug.h 
    ${SRC}/fs.h 
    ${SRC}/net.h 
    ${SRC}/notify.h
)

set(LINUX_TEST_SOURCES
    ${TEST_SRC}/darray_tests.c
    ${TEST_SRC}/hashmap_tests.c
)

set(OSX_SOURCES
    ${SRC}/bstrlib.c
    ${SRC}/collections.c
    ${SRC}/fs.c
    ${SRC}/net.c 
    ${SRC}/events.c
)

set(LINUX_HEADERS
    ${SRC}/bstrlib.h 
    ${SRC}/collections.h 
    ${SRC}/debug.h 
    ${SRC}/fs.h 
    ${SRC}/net.h 
    ${SRC}/events.h
)






# Bring headers
# include_directories(src)

# Use GLOB and wildcards
#file(GLOB SOURCES src/server.c)

#OSX build
# add_executable(client  client.c ${TEST_SOURCES})
add_executable(kevent client.c ${OSX_SOURCES})


#add_executable(testClient osx/client.c osx/client_main.c)

# linux build
#add_executable(testServer src/server.c src/server_main.c)
#add_executable(testClient src/client.c src/client_main.c)
#add_executable(testClient src/client.c)
