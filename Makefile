CC = gcc
CFLAGS = -Wall
RPC_SYSTEM = rpc

OBJECTS = rpc.o

.PHONY: format all

all: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

format:
	clang-format -style=file -i *.c *.h
