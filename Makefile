all: create-bin-dir compile

create-bin-dir:
	mkdir -p bin

compile:
	gcc main.c -g -o bin/zcc