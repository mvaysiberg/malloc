all: mymalloc.o mymalloc.h
	gcc -o memgrind memgrind.c

mymalloc.o: mymalloc.c
	gcc -c mymalloc.c

clean:
	rm memgrind *.o
