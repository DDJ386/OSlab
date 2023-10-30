main: main.o pages.o 
	gcc -o main main.o pages.o

main.o: pages.h main.c
	gcc -c -g main.c

pages.o: pages.h pages.c
	gcc -c -g pages.c

.PHONY:clean
clean: main
	rm -f *.o