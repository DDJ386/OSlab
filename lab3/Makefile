all: shell.o fs.o
	gcc -o fs shell.o fs.o
shell.o: shell.c
	gcc -c -g shell.c
fs.o: fs.c
	gcc -c -g fs.c
