CC=cc
RPC_SYSTEM=rpc.o
CFLANGS = -Wall

.PHONY: format all

all: $(RPC_SYSTEM)

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) $(CFLANGS)-c -o $@ $<


format:
	clang-format -style=file -i *.c *.h
