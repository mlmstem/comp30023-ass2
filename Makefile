CC=gcc
RPC_SYSTEM=rpc.o
CFLANGS = -Wall

.PHONY: format all

all: $(RPC_SYSTEM)

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) $(CFLANGS) -o -g $(RPC_SYSTEM)$@ $<

RPC_SYSTEM_A=rpc.a
$(RPC_SYSTEM_A): rpc.o
	ar rcs $(RPC_SYSTEM_A) 

clean:
	rm *.o *.a all

format:
	clang-format -style=file -i *.c *.h