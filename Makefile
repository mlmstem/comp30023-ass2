CC=gcc
CFLANGS = -Wall

.PHONY: format all

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) $(CFLANGS) rpc.c -o $(RPC_SYSTEM) 

RPC_SYSTEM_A=rpc.a
$(RPC_SYSTEM_A): rpc.o
	ar rcs $(RPC_SYSTEM_A) 

clean:
	rm -f rpc.o

format:
	clang-format -style=file -i *.c *.h