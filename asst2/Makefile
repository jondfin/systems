FLAGS = -Wall -Werror -fsanitize=address

all: multiThreadSorter

multiThreadSorter: multiThreadSorter.o mergesort.o
	gcc $(FLAGS) -lpthread multiThreadSorter.o mergesort.o -o multiThreadSorter -g 

multiThreadSorter.o: multiThreadSorter.c
	gcc $(FLAGS) -c -g multiThreadSorter.c -o multiThreadSorter.o

mergesort.o: mergesort.c
	gcc $(FLAGS) -c -g mergesort.c -o mergesort.o

clean:
	rm *.o multiThreadSorter
