all: create-bin-dir compile

create-bin-dir:
	mkdir -p ../../bin

compile:
	gcc main.c chunk.c debug.c memory.c value.c vm.c -g -o ../../bin/zcc