test1: test1.o pages.o 
	gcc -o test1 test1.o pages.o

test2: test2.o pages.o
	gcc -o test2 test2.o pages.o

test1.o: pages.h test1.c
	gcc -c -g test1.c

test2.o: pages.h test2.c
	gcc -c -g test2.c

pages.o: pages.h pages.c
	gcc -c -g pages.c

.PHONY:clean
clean: test1 test2
	rm -f *.o