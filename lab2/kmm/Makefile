main: main.o kmm.o 
	gcc -o main main.o kmm.o

main.o: kmm.h
	gcc -c main.c

kmm.o: kmm.h
	gcc -c kmm.c

.PHONY:clean
clean: main
	rm -f *.o