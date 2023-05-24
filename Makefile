CC = gcc
CFLAGS = -Wall
RPC_SYSTEM = rpc

OBJECTS = rpc.o

.PHONY: format all

$(RPC_SYSTEM): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(RPC_SYSTEM)

$(OBJECTS): rpc.c rpc.h
	$(CC) $(CFLAGS) -c rpc.c -o $(OBJECTS)

clean:
	rm -f $(RPC_SYSTEM) $(OBJECTS)

format:
	clang-format -style=file -i *.c *.h
