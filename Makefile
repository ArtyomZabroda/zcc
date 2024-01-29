all: create-bin-dir compile

create-bin-dir:
	mkdir bin

compile:
	gcc main.c -o bin/zcc