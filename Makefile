CC=cc
RPC_SYSTEM=rpc.o
CFLANGS = -Wall -g

.PHONY: format all

all: $(RPC_SYSTEM)

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) $(CFLANGS) -o $@ $<

RPC_SYSTEM_A=rpc.a
#$(RPC_SYSTEM_A): rpc.o
	ar rcs $(RPC_SYSTEM_A) $(RPC_SYSTEM)

format:
	clang-format -style=file -i *.c *.h

clean:
    rm *.o all
